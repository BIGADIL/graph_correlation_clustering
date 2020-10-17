#include "../../../../include/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/BranchAndBounds.hpp"

IClustPtr set_semi_supervised_2cc::BranchAndBounds::GetBestClustering(const IGraphPtr &graph,
                                                                      const IClustPtr &initial_clustering,
                                                                      const std::vector<unsigned> &first_cluster_vertices,
                                                                      const std::vector<unsigned> &second_cluster_vertices) {
  graph_ = graph;
  auto clustering = BBBinaryClusteringVector(graph->Size(), graph);
  for (const auto &fcv : first_cluster_vertices) {
    clustering.SetupLabelForVertex(fcv, FIRST_CLUSTER);
  }
  for (const auto &scv: second_cluster_vertices) {
    clustering.SetupLabelForVertex(scv, SECOND_CLUSTER);
  }
  record_ = initial_clustering->GetDistanceToGraph(*graph);
  best_clustering_ = initial_clustering;
  Branch(clustering);
  return best_clustering_;
}

void set_semi_supervised_2cc::BranchAndBounds::Branch(BBBinaryClusteringVector &clustering) {
  auto num_clustered =
      clustering.GetNumVerticesByLabel(FIRST_CLUSTER) + clustering.GetNumVerticesByLabel(SECOND_CLUSTER);
  if (num_clustered != graph_->Size()) {
    auto v = clustering.Choose();
    auto right_clustering = clustering.Copy();
    right_clustering.SetupLabelForVertex(v, FIRST_CLUSTER);
    auto bound = right_clustering.Bound(record_);
    if (bound < int(record_)) {
      Branch(right_clustering);
    }
    auto left_clustering = clustering.Copy();
    left_clustering.SetupLabelForVertex(v, SECOND_CLUSTER);
    bound = left_clustering.Bound(record_);
    if (bound < int(record_)) {
      Branch(left_clustering);
    }
  } else {
    auto bound = clustering.GetDistanceToGraph(*graph_);
    if (bound < record_) {
      record_ = bound;
      best_clustering_ = std::make_shared<BBBinaryClusteringVector>(clustering);
    }
  }
}