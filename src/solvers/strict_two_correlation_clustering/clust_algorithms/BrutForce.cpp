#include <climits>

#include "../../../../include/solvers/strict_two_correlation_clustering/clust_algorithms/BrutForce.hpp"

IClustPtr strict_2cc::BrutForce::GetBestClustering(const IGraphPtr &graph) const {
  unsigned best_clustering = UINT_MAX;
  unsigned best_distance = UINT_MAX;
  unsigned init_clustering = 1;
  const unsigned last_bit = 1U << (graph->Size() - 1);
  while ((init_clustering & last_bit) == 0) {
    if (const auto tmp_distance = GetDistanceToGraph(*graph, init_clustering); tmp_distance < best_distance) {
      best_distance = tmp_distance;
      best_clustering = init_clustering;
    }
    init_clustering++;
  }
  auto result = factory_->CreateClustering(graph->Size());
  for (unsigned i = 0; i < graph->Size(); ++i) {
    if ((best_clustering & (1U << i)) == 0) {
      result->SetupLabelForVertex(i, FIRST_CLUSTER);
    } else {
      result->SetupLabelForVertex(i, SECOND_CLUSTER);
    }
  }
  return result;
}

unsigned strict_2cc::BrutForce::GetDistanceToGraph(const IGraph &graph, const unsigned clustering) {
  unsigned distance = 0;
  for (unsigned i = 0; i < graph.Size(); i++) {
    for (unsigned j = i + 1; j < graph.Size(); j++) {
      const bool first_label = (clustering & (1U << i)) == 0;
      if (const bool second_label = (clustering & (1U << j)) == 0; (first_label != second_label && graph.IsJoined(i, j))
                                                             || (first_label == second_label && !graph.IsJoined(i, j))) {
        distance++;
      }
    }
  }
  return distance;
}

strict_2cc::BrutForce::BrutForce(IClustFactoryPtr factory) :
    factory_(std::move(factory)) {

}
