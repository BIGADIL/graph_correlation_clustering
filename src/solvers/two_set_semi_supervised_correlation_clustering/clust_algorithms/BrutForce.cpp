#include <climits>
#include "../../../../include/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/BrutForce.hpp"

IClustPtr set_semi_supervised_2cc::BrutForce::GetBestClustering(const IGraphPtr &graph,
                                                                const std::vector<unsigned> &first_cluster_vertices,
                                                                const std::vector<unsigned> &second_cluster_vertices) {
  unsigned best_clustering = UINT_MAX;
  unsigned best_distance = UINT_MAX;
  unsigned init_clustering = 0;
  unsigned last_bit = 1U << (graph->Size() - 1);
  while ((init_clustering & last_bit) == 0) {
    if (!IsValidClustering(first_cluster_vertices, second_cluster_vertices, init_clustering)) {
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

unsigned set_semi_supervised_2cc::BrutForce::GetDistanceToGraph(IGraph &graph, unsigned int clustering) {
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

set_semi_supervised_2cc::BrutForce::BrutForce(IClustFactoryPtr factory) :
    factory_(std::move(factory)) {

}
bool set_semi_supervised_2cc::BrutForce::IsValidClustering(const std::vector<unsigned int> &first_cluster_vertices,
                                                           const std::vector<unsigned int> &second_cluster_vertices,
                                                           unsigned int clustering) {
  for (unsigned i = 0; i < first_cluster_vertices.size(); ++i) {
    for (unsigned j = i + 1; j < first_cluster_vertices.size(); ++j) {
      bool first_label = (clustering & (1U << first_cluster_vertices[i])) == 0;
      bool second_label = (clustering & (1U << first_cluster_vertices[j])) == 0;
      if (first_label != second_label) return false;
    }
  }
  for (unsigned i = 0; i < second_cluster_vertices.size(); ++i) {
    for (unsigned j = i + 1; j < second_cluster_vertices.size(); ++j) {
      bool first_label = (clustering & (1U << second_cluster_vertices[i])) == 0;
      bool second_label = (clustering & (1U << second_cluster_vertices[j])) == 0;
      if (first_label != second_label) return false;
    }
  }
  for (unsigned int first_cluster_vertex : first_cluster_vertices) {
    for (unsigned int second_cluster_vertex : second_cluster_vertices) {
      bool first_label = (clustering & (1U << first_cluster_vertex)) == 0;
      bool second_label = (clustering & (1U << second_cluster_vertex)) == 0;
      if (first_label == second_label) return false;
    }
  }
  return true;
}
