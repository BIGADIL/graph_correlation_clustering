#pragma once

#include <vector>

#include "../../../clustering/BBBinaryClusteringVector.hpp"
#include "../../../clustering/IClustering.hpp"

namespace set_semi_supervised_2cc {
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
   * @param first_cluster_vertices
   * @param second_cluster_vertices
   * @return optimal solution for source graph.
   */
  IClustPtr GetBestClustering(
      const IGraphPtr &graph, const IClustPtr &initial_clustering,
      const std::vector<unsigned> &first_cluster_vertices,
      const std::vector<unsigned> &second_cluster_vertices);
};
}  // namespace set_semi_supervised_2cc
