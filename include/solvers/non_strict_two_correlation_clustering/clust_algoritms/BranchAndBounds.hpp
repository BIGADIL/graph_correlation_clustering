#pragma once

#include "../../../clustering/BBBinaryClusteringVector.hpp"
#include "../../../clustering/IClustering.hpp"

namespace non_strict_2cc {
/**
 * Base branch and bound algorithm for NS2CC.
 *
 * It works only with ::BBBinaryClusteringVector.
 */
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
   * @return optimal solution for source graph.
   */
  IClustPtr GetBestClustering(const IGraphPtr &graph,
                              const IClustPtr &initial_clustering);
};
}  // namespace non_strict_2cc
