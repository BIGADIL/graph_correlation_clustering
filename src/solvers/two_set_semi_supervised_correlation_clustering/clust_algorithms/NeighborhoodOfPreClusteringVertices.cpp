#include <algorithm>

#include "../../../../include/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodOfPreClusteringVertices.hpp"
#include "../../../../include/solvers/two_set_semi_supervised_correlation_clustering/common_functions/LocalSearch.hpp"

set_semi_supervised_2cc::NeighborhoodOfPreClusteringVertices::NeighborhoodOfPreClusteringVertices(
    IClustFactoryPtr clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {

}
IClustPtr set_semi_supervised_2cc::NeighborhoodOfPreClusteringVertices::getBestNeighborhoodClustering(
    const IGraph &graph,
    const std::vector<unsigned int> &first_cluster_vertices,
    const std::vector<unsigned int> &second_cluster_vertices) const {
  auto first_clustering = clustering_factory_->CreateClustering(graph.Size());
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if (IsJoinedWithSet(first_cluster_vertices, i, graph) && !IsVertexInSet(i, second_cluster_vertices)) {
      first_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
    } else {
      first_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
    }
  }
  first_clustering =
      LocalSearch::ComputeLocalOptimum(graph, first_clustering, first_cluster_vertices, second_cluster_vertices);
  auto first_distance = first_clustering->GetDistanceToGraph(graph);

  auto second_clustering = clustering_factory_->CreateClustering(graph.Size());
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if (IsJoinedWithSet(second_cluster_vertices, i, graph) && !IsVertexInSet(i, first_cluster_vertices)) {
      second_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
    } else {
      second_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
    }
  }
  second_clustering =
      LocalSearch::ComputeLocalOptimum(graph, second_clustering, first_cluster_vertices, second_cluster_vertices);
  auto second_distance = second_clustering->GetDistanceToGraph(graph);
  return first_distance < second_distance ? first_clustering : second_clustering;
}

bool set_semi_supervised_2cc::NeighborhoodOfPreClusteringVertices::IsJoinedWithSet(const std::vector<unsigned> &set,
                                                                                   const unsigned vertex,
                                                                                   const IGraph &graph) {
  for (const auto &s: set) {
    if (vertex == s || graph.IsJoined(vertex, s)) {
      return true;
    }
  }
  return false;
}
bool set_semi_supervised_2cc::NeighborhoodOfPreClusteringVertices::IsVertexInSet(const unsigned int vertex,
                                                                                 const std::vector<unsigned int> &vertices_set) {
  return std::find(vertices_set.begin(), vertices_set.end(), vertex) != vertices_set.end();
}
