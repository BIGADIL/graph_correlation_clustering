#include "../../include/algorithms/BranchAndBoundAlgorithm.hpp"

std::shared_ptr<IClustering> BranchAndBoundAlgorithm::GetBestClustering(const std::shared_ptr<IGraph> &graph,
                                                                        const std::shared_ptr<IClustering> &clustering_record) {
  graph_ = graph;
  auto clustering = BoundAndBranchBinaryClusteringVector(graph->Size(), graph);
  record_ = clustering_record->GetDistanceToGraph(*graph);
  best_clustering_ = clustering_record;
  Branch(clustering);
  return best_clustering_;
}

void BranchAndBoundAlgorithm::Branch(BoundAndBranchBinaryClusteringVector &clustering) {
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
      best_clustering_ = std::make_shared<BoundAndBranchBinaryClusteringVector>(clustering);
    }
  }
}