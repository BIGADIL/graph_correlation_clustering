#pragma once

#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace non_strict_3cc {

class NeighborSplitter {
 private:
  /**
  * Factory that create new clustering.
  */
  IClustFactoryPtr clustering_factory_;

 public:
  NeighborSplitter(IClustFactoryPtr clustering_factory);

  [[nodiscard]] IClustPtr BuildFirstCluster(const IGraph &graph,
                                            unsigned vertex) const;

  [[nodiscard]] IClustPtr BuildSecondAndThirdClusters(const IGraph &graph,
                                                      const IClustPtr &init_clustering,
                                                      unsigned vertex) const;

  [[nodiscard]] IClustPtr SplitGraphByTwoVertices(const IGraph &graph,
                                                  unsigned first_vertex,
                                                  unsigned second_vertex) const;
};

}

