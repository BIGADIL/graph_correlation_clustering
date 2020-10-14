#pragma once

#include <memory>

#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace non_strict_2cc {

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
    NeighborSplitter(IClustFactoryPtr clustering_factory);
    /**
     * Split source graph by vertex based on its neighborhood.
     *
     * @param graph source graph.
     * @param vertex source vertex.
     * @return clustering based on vertex neighborhood.
     */
    IClustPtr SplitGraphByVertex(const IGraph &graph,
                                 const unsigned vertex) const;
  };
}