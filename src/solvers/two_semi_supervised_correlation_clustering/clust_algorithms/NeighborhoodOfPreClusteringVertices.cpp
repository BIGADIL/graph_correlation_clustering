#include "../../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodOfPreClusteringVertices.hpp"
#include "../../../../include/solvers/two_semi_supervised_correlation_clustering/common_functions/LocalSearch.hpp"

semi_supervised_2cc::NeighborhoodOfPreClusteringVertices::NeighborhoodOfPreClusteringVertices(
    IClustFactoryPtr clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {

}

IClustPtr semi_supervised_2cc::NeighborhoodOfPreClusteringVertices::getBestNeighborhoodClustering(
    const IGraph &graph,
    unsigned int first_cluster_vertex,
    unsigned int second_cluster_vertex) const {
  auto first_clustering = clustering_factory_->CreateClustering(graph.Size());
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if ((i == first_cluster_vertex || graph.IsJoined(i, first_cluster_vertex)) && i != second_cluster_vertex) {
      first_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
    } else {
      first_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
    }
  }
  first_clustering = LocalSearch::ComputeLocalOptimum(graph, first_clustering, first_cluster_vertex, second_cluster_vertex);
  auto first_distance = first_clustering->GetDistanceToGraph(graph);

  auto second_clustering = clustering_factory_->CreateClustering(graph.Size());
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if ((i == second_cluster_vertex || graph.IsJoined(i, second_cluster_vertex)) && i != first_cluster_vertex) {
      second_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
    } else {
      second_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
    }
  }
  second_clustering = LocalSearch::ComputeLocalOptimum(graph, second_clustering, first_cluster_vertex, second_cluster_vertex);
  auto second_distance = second_clustering->GetDistanceToGraph(graph);

  return first_distance < second_distance ? first_clustering : second_clustering;
}
