#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace semi_supervised_2cc {
class BrutForce {
  static unsigned GetDistanceToGraph(const IGraph &graph, unsigned clustering);

  IClustFactoryPtr factory_;

 public:
  explicit BrutForce(IClustFactoryPtr factory);

  [[nodiscard]] IClustPtr GetBestClustering(const IGraphPtr &graph,
                                            unsigned first_vertex,
                                            unsigned second_vertex) const;
};
}  // namespace semi_supervised_2cc
