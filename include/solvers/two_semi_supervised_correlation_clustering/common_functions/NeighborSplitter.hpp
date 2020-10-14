#pragma once

#include <vector>

#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace semi_supervised_2cc {

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
  [[nodiscard]] std::vector<IClustPtr> SplitGraphByVertex(const IGraph &graph,
                                                          unsigned vertex,
                                                          unsigned first_cluster_vertex,
                                                          unsigned second_cluster_vertex) const;
};

}
