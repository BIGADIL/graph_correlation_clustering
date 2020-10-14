#include <thread>
#include <climits>

#include "../../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/N1LSAlgorithmFor2SCC.hpp"
#include "../../../../include/solvers/two_semi_supervised_correlation_clustering/common_functions/LSAlgorithmFor2SCC.hpp"

IClustPtr N1LSAlgorithmFor2SCC::getBestNeighborhoodClustering(const IGraph &graph,
                                                              const unsigned first_cluster_vertex,
                                                              const unsigned second_cluster_vertex) const {
  std::vector<IClustPtr> local_best_clustering_vector;
  for (unsigned i = 0; i < num_threads_; i++) {
    auto instance = clustering_factory_->CreateClustering(graph.Size());
    local_best_clustering_vector.push_back(instance);
  }
  std::vector<std::thread> thread_vector(num_threads_);
  for (unsigned i = 0; i < num_threads_; i++) {
    thread_vector[i] = std::thread(
        &N1LSAlgorithmFor2SCC::BestNeighborhoodClusteringThreadWorker,
        this,
        std::ref(graph),
        i,
        std::ref(local_best_clustering_vector[i]),
        first_cluster_vertex,
        second_cluster_vertex
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
  return LSAlgorithmFor2SCC::ComputeLocalOptimum(
      graph,
      best_neighborhood_clustering,
      first_cluster_vertex,
      second_cluster_vertex);
}

void N1LSAlgorithmFor2SCC::BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                                                  const unsigned threadId,
                                                                  IClustPtr &local_best_clustering,
                                                                  const unsigned first_cluster_vertex,
                                                                  const unsigned second_cluster_vertex) const {
  unsigned best_distance = UINT_MAX;
  for (unsigned i = threadId; i < graph.Size(); i += num_threads_) {
    auto tmp_neighborhood_clustering = neighbor_splitter_.SplitGraphByVertex(
        graph,
        i,
        first_cluster_vertex,
        second_cluster_vertex);
    for (auto &clustering: tmp_neighborhood_clustering) {
      unsigned tmp_distance = clustering->GetDistanceToGraph(graph);
      if (tmp_distance < best_distance) {
        best_distance = tmp_distance;
        local_best_clustering = clustering;
      }
    }
  }
}

N1LSAlgorithmFor2SCC::N1LSAlgorithmFor2SCC(const unsigned num_threads,
                                           const IClustFactoryPtr &clustering_factory)
    : num_threads_(num_threads),
      clustering_factory_(clustering_factory),
      neighbor_splitter_(NSplitterFor2SCC(clustering_factory)) {
}