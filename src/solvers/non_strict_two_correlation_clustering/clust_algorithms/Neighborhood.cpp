#include <vector>
#include <thread>
#include <climits>

#include "../../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/Neighborhood.hpp"

IClustPtr non_strict_2cc::Neighborhood::getBestNeighborhoodClustering(const IGraph &graph) const {
  std::vector<Solution> result = getAllSolutions(graph);
  unsigned best_distance = UINT_MAX;
  IClustPtr best_clustering = nullptr;
  for (auto &it: result) {
    if (it.distance < best_distance) {
      best_distance = it.distance;
      best_clustering = it.clustering;
    }
  }
  return best_clustering;
}

void non_strict_2cc::Neighborhood::BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                                                          const unsigned threadId,
                                                                          std::vector<Solution> &local_thread_buffer) const {
  for (unsigned i = threadId; i < graph.Size(); i += num_threads_) {
    auto tmp_neighborhood_clustering = neighbor_splitter_.SplitGraphByVertex(graph, i);
    unsigned tmp_distance = tmp_neighborhood_clustering->GetDistanceToGraph(graph);
    local_thread_buffer.emplace_back(Solution(tmp_distance, tmp_neighborhood_clustering));
  }
}

non_strict_2cc::Neighborhood::Neighborhood(const unsigned num_threads,
                                           const IClustFactoryPtr &clustering_factory)
    : num_threads_(num_threads),
      clustering_factory_(clustering_factory),
      neighbor_splitter_(non_strict_2cc::NeighborSplitter(clustering_factory)) {
}

std::vector<Solution> non_strict_2cc::Neighborhood::getAllSolutions(const IGraph &graph) const {
  std::vector<std::vector<Solution>> local_thread_buffer;
  for (unsigned i = 0; i < num_threads_; i++) {
    local_thread_buffer.emplace_back(std::vector<Solution>());
  }
  std::vector<std::thread> thread_vector(num_threads_);
  for (unsigned i = 0; i < num_threads_; i++) {
    thread_vector[i] = std::thread(
        &Neighborhood::BestNeighborhoodClusteringThreadWorker,
        this,
        std::ref(graph),
        i,
        std::ref(local_thread_buffer[i])
    );
  }
  for (auto &it: thread_vector) {
    it.join();
  }

  std::vector<Solution> result;
  for (auto &it: local_thread_buffer) {
    for (auto &elem: it) {
      result.emplace_back(elem);
    }
  }
  return result;
}