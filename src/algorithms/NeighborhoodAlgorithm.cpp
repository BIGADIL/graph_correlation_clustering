#include <climits>
#include <thread>
#include <utility>

#include "../../include/algorithms/NeighborhoodAlgorithm.hpp"
#include "../../include/clustering/BinaryClusteringVector.hpp"

std::shared_ptr<IClustering> NeighborhoodAlgorithm::getBestNeighborhoodClustering(const IGraph &graph) const {
  std::vector<IClusteringPointer> local_best_neighborhood_clustering_vector;
  for (unsigned i = 0; i < num_threads_; i++) {
    auto instance = clustering_factory_->CreateClustering(graph.Size());
    local_best_neighborhood_clustering_vector.push_back(instance);
  }
  std::vector<std::thread> thread_vector(num_threads_);

  for (unsigned i = 0; i < num_threads_; i++) {
    thread_vector[i] = std::thread(
        &NeighborhoodAlgorithm::BestNeighborhoodClusteringThreadWorker,
        this,
        std::ref(graph),
        i,
        std::ref(local_best_neighborhood_clustering_vector[i])
    );
  }

  for (auto &it: thread_vector) {
    it.join();
  }

  IClusteringPointer best_neighborhood_clustering = local_best_neighborhood_clustering_vector[0];
  unsigned best_distance = best_neighborhood_clustering->GetDistanceToGraph(graph);
  for (auto &it: local_best_neighborhood_clustering_vector) {
    auto tmp_distance = it->GetDistanceToGraph(graph);
    if (tmp_distance < best_distance) {
      best_distance = tmp_distance;
      best_neighborhood_clustering = it;
    }
  }
  return best_neighborhood_clustering;
}

void NeighborhoodAlgorithm::BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                                                   const unsigned threadId,
                                                                   IClusteringPointer &local_best_neighborhood_clustering) const {
  unsigned best_distance = UINT_MAX;
  for (unsigned i = threadId; i < graph.Size(); i += num_threads_) {
    auto tmp_neighborhood_clustering = neighbor_splitter_.SplitGraphByVertex(graph, i);
    tmp_neighborhood_clustering = local_search_algorithm_.ComputeLocalOptimum(graph, tmp_neighborhood_clustering);
    unsigned tmp_distance = tmp_neighborhood_clustering->GetDistanceToGraph(graph);
    if (tmp_distance < best_distance) {
      best_distance = tmp_distance;
      local_best_neighborhood_clustering = std::move(tmp_neighborhood_clustering);
    }
  }
}

NeighborhoodAlgorithm::NeighborhoodAlgorithm(const unsigned num_threads,
                                             const std::shared_ptr<IClusteringFactory> &clustering_factory)
    : num_threads_(num_threads),
      clustering_factory_(clustering_factory),
      neighbor_splitter_(NeighborSplitter(clustering_factory)),
      local_search_algorithm_(LocalSearchAlgorithm(clustering_factory)) {
}
