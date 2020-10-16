#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
namespace non_strict_3cc {

class BrutForce {
 private:
  static unsigned GetDistanceToGraph(IGraph &graph, const std::vector<int>& clustering);

  IClustFactoryPtr factory_;

 public:
  explicit BrutForce(const IClustFactoryPtr factory);
  /**
   * Get optimal solution for source graph.
   * @param graph source graph.
   * @return optimal solution for source graph.
   */

  IClustPtr GetBestClustering(const IGraphPtr &graph);
};

}
