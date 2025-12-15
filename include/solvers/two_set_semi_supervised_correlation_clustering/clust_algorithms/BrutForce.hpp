#pragma once

#include <vector>

#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace set_semi_supervised_2cc {
class BrutForce {
  static bool IsValidClustering(
      const std::vector<unsigned> &first_cluster_vertices,
      const std::vector<unsigned> &second_cluster_vertices,
      unsigned clustering);

  static unsigned GetDistanceToGraph(const IGraph &graph, unsigned clustering);

  IClustFactoryPtr factory_;

 public:
  explicit BrutForce(IClustFactoryPtr factory);

  [[nodiscard]] IClustPtr GetBestClustering(
      const IGraphPtr &graph,
      const std::vector<unsigned> &first_cluster_vertices,
      const std::vector<unsigned> &second_cluster_vertices) const;
};
}  // namespace set_semi_supervised_2cc
