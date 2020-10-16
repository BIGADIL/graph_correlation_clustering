#pragma once

#include "../../../clustering/IClustering.hpp"
namespace non_strict_3cc {

class BrutForce {
 public:
  explicit BrutForce() = default;
  /**
   * Get optimal solution for source graph.
   * @param graph source graph.
   * @return optimal solution for source graph.
   */

  static IClustPtr GetBestClustering(const IGraphPtr &graph);
};

}
