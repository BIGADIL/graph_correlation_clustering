#include <vector>
#include <thread>
#include <climits>

#include "../../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithOneLocalSearch.hpp"
#include "../../../../include/solvers/non_strict_two_correlation_clustering/common_functions/LocalSearch.hpp"

IClustPtr non_strict_2cc::NeighborhoodWithOneLocalSearch::getBestNeighborhoodClustering(const IGraph &graph) const {
  std::vector<IClustPtr> local_best_clustering_vector;
  for (unsigned i = 0; i < num_threads_; i++) {
    auto instance = clustering_factory_->CreateClustering(graph.Size());
    local_best_clustering_vector.push_back(instance);
  }
  std::vector<std::thread> thread_vector(num_threads_);
  for (unsigned i = 0; i < num_threads_; i++) {
    thread_vector[i] = std::thread(
        &NeighborhoodWithOneLocalSearch::BestNeighborhoodClusteringThreadWorker,
        this,
        std::ref(graph),
        i,
        std::ref(local_best_clustering_vector[i])
    );
  }
  for (auto &it: thread_vector) {
    it.join();
  }
  IClustPtr best_neighborhood_clustering = local_best_clustering_vector[0];
  unsigned best_distance = best_neighborhood_clustering->GetDistanceToGraph(graph);
  for (auto &it: local_best_clustering_vector) {
    auto tmp_distance = it->GetDistanceToGraph(graph);
    if (tmp_distance < best_distance) {
      best_distance = tmp_distance;
      best_neighborhood_clustering = it;
    }
  }
  return LocalSearch::ComputeLocalOptimum(graph, best_neighborhood_clustering);
}

void non_strict_2cc::NeighborhoodWithOneLocalSearch::BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                                                                            const unsigned threadId,
                                                                                            IClustPtr &local_best_clustering) const {
  unsigned best_distance = UINT_MAX;
  for (unsigned i = threadId; i < graph.Size(); i += num_threads_) {
    auto tmp_neighborhood_clustering = neighbor_splitter_.SplitGraphByVertex(graph, i);
    unsigned tmp_distance = tmp_neighborhood_clustering->GetDistanceToGraph(graph);
    if (tmp_distance < best_distance) {
      best_distance = tmp_distance;
      local_best_clustering = std::move(tmp_neighborhood_clustering);
    }
  }
}

non_strict_2cc::NeighborhoodWithOneLocalSearch::NeighborhoodWithOneLocalSearch(const unsigned num_threads,
                                                                               const IClustFactoryPtr &clustering_factory)
    : num_threads_(num_threads),
      clustering_factory_(clustering_factory),
      neighbor_splitter_(NeighborSplitter(clustering_factory)) {
}
