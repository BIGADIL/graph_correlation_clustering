#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/BranchAndBounds.hpp"

IClustPtr non_strict_3cc::BranchAndBounds::GetBestClustering(const IGraphPtr &graph,
                                                             const IClustPtr &initial_clustering) {
  graph_ = graph;
  auto clustering = BBTernaryClusteringVector(graph->Size(), graph);
  clustering.SetupLabelForVertex(0, FIRST_CLUSTER);
  record_ = initial_clustering->GetDistanceToGraph(*graph);
  best_clustering_ = initial_clustering;
  Branch(
      clustering,
      std::vector<ClusterLabels>({FIRST_CLUSTER}),
      std::vector<ClusterLabels>({SECOND_CLUSTER, THIRD_CLUSTER})
      );
  return best_clustering_;
}

void non_strict_3cc::BranchAndBounds::Branch(BBTernaryClusteringVector &clustering,
                                             std::vector<ClusterLabels> used_labels,
                                             std::vector<ClusterLabels> not_used_labels) {
  auto num_clustered = clustering.GetNumVerticesByLabel(FIRST_CLUSTER) +
      clustering.GetNumVerticesByLabel(SECOND_CLUSTER) +
      clustering.GetNumVerticesByLabel(THIRD_CLUSTER);
  if (num_clustered != graph_->Size()) {
    auto v = clustering.Choose();
    for (const auto &next_label: used_labels) {
      auto tmp_clustering = clustering.Copy();
      tmp_clustering.SetupLabelForVertex(v, next_label);
      auto bound = tmp_clustering.Bound(record_);
      if (bound < int(record_)) {
        Branch(tmp_clustering, used_labels, not_used_labels);
      }
    }
    if (!not_used_labels.empty()) {
      auto next_used_label = not_used_labels.back();
      not_used_labels.pop_back();
      auto tmp_clustering = clustering.Copy();
      tmp_clustering.SetupLabelForVertex(v, next_used_label);
      used_labels.push_back(next_used_label);
      auto bound = tmp_clustering.Bound(record_);
      if (bound < int(record_)) {
        Branch(tmp_clustering, used_labels, not_used_labels);
      }
    }
  } else {
    auto bound = clustering.GetDistanceToGraph(*graph_);
    if (bound < record_) {
      record_ = bound;
      best_clustering_ = std::make_shared<BBTernaryClusteringVector>(clustering);
    }
  }
}