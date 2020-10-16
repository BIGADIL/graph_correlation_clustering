#pragma once

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace strict_2cc {

class BrutForce {
 private:
  static unsigned GetDistanceToGraph(IGraph &graph, unsigned clustering);

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
