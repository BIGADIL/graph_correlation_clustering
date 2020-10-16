#include <climits>
#include "../../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BrutForce.hpp"
#include "../../../../include/clustering/BFBinaryClusteringVector.hpp"

IClustPtr semi_supervised_2cc::BrutForce::GetBestClustering(const IGraphPtr &graph,
                                                            const unsigned int first_vertex,
                                                            const unsigned int second_vertex) {
  BFBinaryClusteringVector tmp_clustering(graph->Size());
  IClustPtr best_clustering = nullptr;
  unsigned best_distance = UINT_MAX;
  auto next_clustering = tmp_clustering.getNextClustering();
  while (next_clustering != nullptr) {
    if (next_clustering->IsSameClustered(first_vertex, second_vertex)) {
      next_clustering = tmp_clustering.getNextClustering();
      continue;
    }
    auto tmp_distance = next_clustering->GetDistanceToGraph(*graph);
    if (tmp_distance < best_distance) {
      best_distance = tmp_distance;
      best_clustering = next_clustering;
    }
    next_clustering = tmp_clustering.getNextClustering();
  }
  return best_clustering;
}