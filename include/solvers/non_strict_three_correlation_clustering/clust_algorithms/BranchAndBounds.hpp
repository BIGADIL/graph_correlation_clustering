#pragma once

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/BBTripleClusteringVector.hpp"

namespace non_strict_3cc {

class BranchAndBounds {
 private:
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

 private:
  /**
   * Branch out the range of feasible solutions.
   * @param clustering current vector of labels.
   */
  void Branch(BBTripleClusteringVector &clustering,
              std::vector<ClusterLabels> used_labels,
              std::vector<ClusterLabels> not_used_labels);

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

}
