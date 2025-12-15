#pragma once

#include <vector>

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace set_semi_supervised_2cc {
class NeighborSplitter {
  /**
   * Factory that create new clustering.
   */
  IClustFactoryPtr clustering_factory_;

  static bool IsVertexInSet(unsigned vertex,
                            const std::vector<unsigned> &vertices_set);

 public:
  explicit NeighborSplitter(IClustFactoryPtr clustering_factory);

  /**
   * Split source graph by vertex based on its neighborhood.
   *
   * @param graph source graph.
   * @param vertex source vertex.
   * @param first_cluster_vertices
   * @param second_cluster_vertices
   * @return clustering based on vertex neighborhood.
   */
  [[nodiscard]] std::vector<IClustPtr> SplitGraphByVertex(
      const IGraph &graph, unsigned vertex,
      const std::vector<unsigned> &first_cluster_vertices,
      const std::vector<unsigned> &second_cluster_vertices) const;
};
}  // namespace set_semi_supervised_2cc
