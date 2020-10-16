#pragma once

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"

namespace semi_supervised_2cc {

class BrutForce {
 public:
  static IClustPtr GetBestClustering(const IGraphPtr &graph,
                                     const unsigned first_vertex,
                                     const unsigned second_vertex);
};

}
