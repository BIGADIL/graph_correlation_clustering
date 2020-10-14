#pragma once

#include <memory>

#include "../../../clustering/factories/IClusteringFactory.hpp"

/**
 * Graph splitter by neighbor.
 * @see Bansal, Blum & Chawla. Correlation Clustering.
 */
class NSplitterForS2CC {
 private:
  /**
  * Factory that create new clustering.
  */
  IClustFactoryPtr clustering_factory_;

 public:
  explicit NSplitterForS2CC(IClustFactoryPtr clustering_factory);
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


