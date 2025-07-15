#include <climits>
#include <vector>
#include <thread>

#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhood.hpp"

void non_strict_3cc::TwoVerticesNeighborhood::BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                                                                     unsigned threadId,
                                                                                     std::vector<Solution> &local_thread_buffer) const {
  for (unsigned i = threadId; i < graph.Size(); i += num_threads_) {
    for (unsigned j = 0; j < graph.Size(); ++j) {
      if (i == j) continue;
      auto tmp_clustering = neighbor_splitter_.SplitGraphByTwoVertices(graph, i, j);
      auto tmp_distance = tmp_clustering->GetDistanceToGraph(graph);
      local_thread_buffer.emplace_back(tmp_distance, tmp_clustering);
    }
  }
}

non_strict_3cc::TwoVerticesNeighborhood::TwoVerticesNeighborhood(unsigned num_threads,
                                                                 const IClustFactoryPtr &clustering_factory) :
    num_threads_(num_threads),
    clustering_factory_(clustering_factory),
    neighbor_splitter_(non_strict_3cc::NeighborSplitter(clustering_factory)) {

}

IClustPtr non_strict_3cc::TwoVerticesNeighborhood::getBestNeighborhoodClustering(const IGraph &graph) const {
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
std::vector<Solution> non_strict_3cc::TwoVerticesNeighborhood::getAllSolutions(const IGraph &graph) const {
  std::vector<std::vector<Solution>> local_thread_buffer;
  for (unsigned i = 0; i < num_threads_; i++) {
    local_thread_buffer.emplace_back();
  }
  std::vector<std::thread> thread_vector(num_threads_);
  for (unsigned i = 0; i < num_threads_; i++) {
    thread_vector[i] = std::thread(
        &TwoVerticesNeighborhood::BestNeighborhoodClusteringThreadWorker,
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