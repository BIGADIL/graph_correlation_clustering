#include <climits>
#include "../../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/BrutForce.hpp"
#include "../../../../include/clustering/BFBinaryClusteringVector.hpp"
#include "../../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BrutForce.hpp"
#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/BrutForce.hpp"

IClustPtr non_strict_2cc::BrutForce::GetBestClustering(const IGraphPtr &graph) {
  BFBinaryClusteringVector tmp_clustering(graph->Size());
  IClustPtr best_clustering = nullptr;
  unsigned best_distance = UINT_MAX;
  auto next_clustering = tmp_clustering.getNextClustering();
  while (next_clustering != nullptr) {
    auto tmp_distance = next_clustering->GetDistanceToGraph(*graph);
    if (tmp_distance < best_distance) {
      best_distance = tmp_distance;
      best_clustering = next_clustering;
    }
    next_clustering = tmp_clustering.getNextClustering();
  }
  return best_clustering;
}
