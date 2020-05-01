#include "../../include/algorithms/BranchAndBoundAlgorithm.hpp"

unsigned BranchAndBoundAlgorithm::GetBestClustering(const std::shared_ptr<IGraph> &graph, int record) {
  graph_ = graph;
  auto clustering = BoundAndBranchBinaryClusteringVector(graph->Size(), graph);
  record_ = record;
  Branch(clustering);
  return record_;
}

void BranchAndBoundAlgorithm::Branch(const BoundAndBranchBinaryClusteringVector &clustering) {
  auto num_clustered = graph_->Size() - clustering.GetNumNonClusteredVertices();
  if (num_clustered != graph_->Size()) {
    auto v = clustering.Choose();
    auto right_clustering = clustering.copy();
    right_clustering.SetupLabelForVertex(v, FIRST_CLUSTER);
    auto bound = right_clustering.Bound(record_);
    if (bound < int(record_)) {
      Branch(right_clustering);
    }
    auto left_clustering = clustering.copy();
    left_clustering.SetupLabelForVertex(v, SECOND_CLUSTER);
    bound = left_clustering.Bound(record_);
    if (bound < int(record_)) {
      Branch(left_clustering);
    }
  } else {
    auto bound = clustering.GetDistanceToGraph(*graph_);
    if (bound < record_) {
      record_ = bound;
    }
  }
}