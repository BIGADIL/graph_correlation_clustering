#pragma once

#include "../../../clustering/BBBinaryClusteringVector.hpp"

namespace semi_supervised_2cc {
class BranchAndBounds {
  /**
   * The source graph to be clustered.
   */
  IGraphPtr graph_;
  /**
   * Record value.
   * @see ::GetBestClustering
   */
  unsigned record_{0};
  /**
   * Best clustering.
   * @see ::GetBestClustering
   */
  IClustPtr best_clustering_ = nullptr;

  /**
   * Branch out the range of feasible solutions.
   * @param clustering current vector of labels.
   */
  void Branch(BBBinaryClusteringVector &clustering);

 public:
  explicit BranchAndBounds() = default;

  /**
   * Get optimal solution for source graph.
   * @param graph source graph.
   * @param initial_clustering initial clustering.
   * @param first_cluster_vertex
   * @param second_cluster_vertex
   * @return optimal solution for source graph.
   */
  IClustPtr GetBestClustering(const IGraphPtr &graph,
                              const IClustPtr &initial_clustering,
                              unsigned first_cluster_vertex,
                              unsigned second_cluster_vertex);
};
}  // namespace semi_supervised_2cc
