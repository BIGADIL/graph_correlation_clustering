#pragma once

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace semi_supervised_2cc {

class BrutForce {
 private:
  static unsigned GetDistanceToGraph(IGraph &graph, unsigned clustering);

  IClustFactoryPtr factory_;

 public:
  explicit BrutForce(IClustFactoryPtr factory);

  IClustPtr GetBestClustering(const IGraphPtr &graph,
                              unsigned first_vertex,
                              unsigned second_vertex);
};

}
