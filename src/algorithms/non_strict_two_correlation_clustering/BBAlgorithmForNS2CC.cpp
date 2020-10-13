#include "../../../include/algorithms/non_strict_two_correlation_clustering/BBAlgorithmForNS2CC.hpp"

IClustPtr BBAlgorithmForNS2CC::GetBestClustering(const IGraphPtr &graph,
                                                 const IClustPtr &initial_clustering) {
  graph_ = graph;
  auto clustering = BBBinaryClusteringVector(graph->Size(), graph);
  record_ = initial_clustering->GetDistanceToGraph(*graph);
  best_clustering_ = initial_clustering;
  Branch(clustering);
  return best_clustering_;
}

void BBAlgorithmForNS2CC::Branch(BBBinaryClusteringVector &clustering) {
  auto num_clustered = graph_->Size() - clustering.GetNumNonClusteredVertices();
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
