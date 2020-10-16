#pragma once

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"

namespace strict_2cc {

class BrutForce {
 public:
  static IClustPtr GetBestClustering(const IGraphPtr &graph);
};

}
