#include <climits>
#include <iostream>

#include "../../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/BrutForce.hpp"

IClustPtr non_strict_2cc::BrutForce::GetBestClustering(const IGraphPtr &graph) {
  unsigned best_clustering = UINT_MAX;
  unsigned best_distance = UINT_MAX;
  unsigned init_clustering = 0;
  unsigned last_bit = 1U << (graph->Size() - 1);
  while ((init_clustering & last_bit) == 0) {
    auto tmp_distance = GetDistanceToGraph(*graph, init_clustering);
    if (tmp_distance < best_distance) {
      best_distance = tmp_distance;
      best_clustering = init_clustering;
    }
    init_clustering++;
  }
  auto result = factory_->CreateClustering(graph->Size());
  for (unsigned i = 0; i < graph->Size(); ++i) {
    auto label = (best_clustering & (1U << i)) == 0;
    if (label) {
      result->SetupLabelForVertex(i, FIRST_CLUSTER);
    } else {
      result->SetupLabelForVertex(i, SECOND_CLUSTER);
    }
  }
  return result;
}

unsigned non_strict_2cc::BrutForce::GetDistanceToGraph(IGraph &graph, unsigned clustering) {
  unsigned distance = 0;
  for (unsigned i = 0; i < graph.Size(); i++) {
    for (unsigned j = i + 1; j < graph.Size(); j++) {
      bool first_label = (clustering & (1U << i)) == 0;
      bool second_label = (clustering & (1U << j)) == 0;
      if ((first_label != second_label && graph.IsJoined(i, j))
          || (first_label == second_label && !graph.IsJoined(i, j))) {
        distance++;
      }
    }
  }
  return distance;
}

non_strict_2cc::BrutForce::BrutForce(IClustFactoryPtr factory) :
    factory_(std::move(factory)) {

}
