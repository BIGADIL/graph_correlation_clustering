#pragma once

#include <vector>

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace set_semi_supervised_2cc {

class BrutForce {
 private:
  static bool IsValidClustering(const std::vector<unsigned> &first_cluster_vertices,
                                const std::vector<unsigned> &second_cluster_vertices,
                                unsigned clustering);

  static unsigned GetDistanceToGraph(IGraph &graph, unsigned clustering);

  IClustFactoryPtr factory_;

 public:
  explicit BrutForce(const IClustFactoryPtr factory);

  IClustPtr GetBestClustering(const IGraphPtr &graph,
                              const std::vector<unsigned> &first_cluster_vertices,
                              const std::vector<unsigned> &second_cluster_vertices);
};

}
