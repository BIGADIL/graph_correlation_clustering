#include "../../../../include/solvers/strict_two_correlation_clustering/clust_algorithms/BBAlgorithmForS2CC.hpp"

IClustPtr BBAlgorithmForS2CC::GetBestClustering(const IGraphPtr &graph,
                                                const IClustPtr &initial_clustering) {
  graph_ = graph;
  auto clustering = BBBinaryClusteringVector(graph->Size(), graph);
  clustering.SetupLabelForVertex(0, FIRST_CLUSTER);
  record_ = initial_clustering->GetDistanceToGraph(*graph);
  best_clustering_ = initial_clustering;
  Branch(clustering);
  return best_clustering_;
}

void BBAlgorithmForS2CC::Branch(BBBinaryClusteringVector &clustering) {
  auto num_vertices_in_first_cluster = clustering.GetNumVerticesByLabel(FIRST_CLUSTER);
  auto num_vertices_in_second_cluster = clustering.GetNumVerticesByLabel(SECOND_CLUSTER);
  auto graph_size = graph_->Size();
  if (num_vertices_in_first_cluster + num_vertices_in_second_cluster != graph_size) {
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
    if (bound < record_ && num_vertices_in_first_cluster != graph_size) {
      record_ = bound;
      best_clustering_ = std::make_shared<BBBinaryClusteringVector>(clustering);
    }
  }
}
