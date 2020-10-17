#include <climits>
#include <thread>
#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/NeighborhoodWith2LocalSearches.hpp"
#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/LocalSearch.hpp"

void non_strict_3cc::NeighborhoodWith2LocalSearches::BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                                                                            unsigned threadId,
                                                                                            IClustPtr &local_best_clustering) const {
  unsigned best_distance = UINT_MAX;
  for (unsigned i = threadId; i < graph.Size(); i += num_threads_) {
    auto part_clustering = neighbor_splitter_.BuildFirstCluster(graph, i);
    for (unsigned k = 0; k < graph.Size(); ++k) {
      if (part_clustering->IsNonClustered(k)) part_clustering->SetupLabelForVertex(k, SECOND_CLUSTER);
    }
    part_clustering = LocalSearch::ComputeLocalOptimum(
        graph,
        part_clustering,
        FIRST_CLUSTER,
        SECOND_CLUSTER
    );
    for (unsigned j = 0; j < graph.Size(); ++j) {
      if (part_clustering->GetLabel(j) == FIRST_CLUSTER) continue;
      auto tmp_clustering = neighbor_splitter_.BuildSecondAndThirdClusters(graph, part_clustering, j);
      tmp_clustering = LocalSearch::ComputeLocalOptimum(
          graph,
          tmp_clustering,
          SECOND_CLUSTER,
          THIRD_CLUSTER);
      auto tmp_distance = tmp_clustering->GetDistanceToGraph(graph);
      if (tmp_distance < best_distance) {
        best_distance = tmp_distance;
        local_best_clustering = tmp_clustering;
      }
    }
  }
}

non_strict_3cc::NeighborhoodWith2LocalSearches::NeighborhoodWith2LocalSearches(unsigned num_threads,
                                                                               const IClustFactoryPtr &clustering_factory)
    :
    num_threads_(num_threads),
    clustering_factory_(clustering_factory),
    neighbor_splitter_(non_strict_3cc::NeighborSplitter(clustering_factory)) {

}

IClustPtr non_strict_3cc::NeighborhoodWith2LocalSearches::getBestNeighborhoodClustering(const IGraph &graph) const {
  std::vector<IClustPtr> local_best_clustering_vector;
  for (unsigned i = 0; i < num_threads_; i++) {
    auto instance = clustering_factory_->CreateClustering(graph.Size());
    local_best_clustering_vector.push_back(instance);
  }
  std::vector<std::thread> thread_vector(num_threads_);
  for (unsigned i = 0; i < num_threads_; i++) {
    thread_vector[i] = std::thread(
        &NeighborhoodWith2LocalSearches::BestNeighborhoodClusteringThreadWorker,
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
  return best_neighborhood_clustering;
}
