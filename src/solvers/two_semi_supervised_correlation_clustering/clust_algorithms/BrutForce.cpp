#include <climits>

#include "../../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BrutForce.hpp"

IClustPtr semi_supervised_2cc::BrutForce::GetBestClustering(const IGraphPtr &graph,
                                                            const unsigned int first_vertex,
                                                            const unsigned int second_vertex) {
  unsigned best_clustering = UINT_MAX;
  unsigned best_distance = UINT_MAX;
  unsigned init_clustering = 0;
  unsigned last_bit = 1U << (graph->Size() - 1);
  while ((init_clustering & last_bit) == 0) {
    auto first_vertex_label = (init_clustering & (1U << first_vertex)) == 0;
    auto second_vertex_label = (init_clustering & (1U << second_vertex)) == 0;
    if (first_vertex_label == second_vertex_label) {
      init_clustering++;
      continue;
    }
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

unsigned semi_supervised_2cc::BrutForce::GetDistanceToGraph(IGraph &graph, unsigned int clustering) {
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

semi_supervised_2cc::BrutForce::BrutForce(IClustFactoryPtr factory) :
    factory_(std::move(factory)) {

}