#include "../../../../include/solvers/strict_two_correlation_clustering/clust_algorithms/BranchAndBounds.hpp"

IClustPtr strict_2cc::BranchAndBounds::GetBestClustering(const IGraphPtr &graph,
                                                         const IClustPtr &initial_clustering) {
  graph_ = graph;
  auto clustering = BBBinaryClusteringVector(graph->Size(), graph);
  clustering.SetupLabelForVertex(0, FIRST_CLUSTER);
  record_ = initial_clustering->GetDistanceToGraph(*graph);
  best_clustering_ = initial_clustering;
  Branch(clustering);
  return best_clustering_;
}

void strict_2cc::BranchAndBounds::Branch(BBBinaryClusteringVector &clustering) {
  const auto num_vertices_in_first_cluster = clustering.GetNumVerticesByLabel(FIRST_CLUSTER);
  const auto num_vertices_in_second_cluster = clustering.GetNumVerticesByLabel(SECOND_CLUSTER);
  if (const auto graph_size = graph_->Size();
    num_vertices_in_first_cluster + num_vertices_in_second_cluster != graph_size) {
    const auto v = clustering.Choose();
    auto right_clustering = clustering.Copy();
    right_clustering.SetupLabelForVertex(v, FIRST_CLUSTER);
    auto bound = right_clustering.Bound(record_);
    if (bound < static_cast<int>(record_)) {
      Branch(right_clustering);
    }
    auto left_clustering = clustering.Copy();
    left_clustering.SetupLabelForVertex(v, SECOND_CLUSTER);
    bound = left_clustering.Bound(record_);
    if (bound < static_cast<int>(record_)) {
      Branch(left_clustering);
    }
  } else {
    if (const auto bound = clustering.GetDistanceToGraph(*graph_);
      bound < record_ && num_vertices_in_first_cluster != graph_size) {
      record_ = bound;
      best_clustering_ = std::make_shared<BBBinaryClusteringVector>(clustering);
    }
  }
}
