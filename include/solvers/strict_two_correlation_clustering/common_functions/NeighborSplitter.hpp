#pragma once

#include <memory>

#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace strict_2cc {
/**
 * Graph splitter by neighbor.
 * @see Bansal, Blum & Chawla. Correlation Clustering.
 */
class NeighborSplitter {
 private:
  /**
  * Factory that create new clustering.
  */
  IClustFactoryPtr clustering_factory_;

 public:
  explicit NeighborSplitter(IClustFactoryPtr clustering_factory);
  /**
   * Split source graph by vertex based on its neighborhood.
   *
   * @param graph source graph.
   * @param vertex source vertex.
   * @return clustering based on vertex neighborhood.
   */
  [[nodiscard]] IClustPtr SplitGraphByVertex(const IGraph &graph,
                                             unsigned vertex,
                                             unsigned opposite_vertex) const;
};

}
