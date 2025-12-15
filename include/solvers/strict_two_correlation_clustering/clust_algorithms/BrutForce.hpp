#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace strict_2cc {

class BrutForce {
  static unsigned GetDistanceToGraph(const IGraph &graph, unsigned clustering);

  IClustFactoryPtr factory_;

 public:
  explicit BrutForce(IClustFactoryPtr factory);
  /**
   * Get optimal solution for source graph.
   * @param graph source graph.
   * @return optimal solution for source graph.
   */

  [[nodiscard]] IClustPtr GetBestClustering(const IGraphPtr &graph) const;
};

}  // namespace strict_2cc
