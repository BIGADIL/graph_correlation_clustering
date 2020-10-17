#pragma once

#include "../../../clustering/factories/IClusteringFactory.hpp"
namespace semi_supervised_2cc {

class NeighborhoodOfPreClusteringVertices {
 public:
  NeighborhoodOfPreClusteringVertices() = delete;
  NeighborhoodOfPreClusteringVertices(const NeighborhoodOfPreClusteringVertices &&) = delete;
  NeighborhoodOfPreClusteringVertices &operator=(const NeighborhoodOfPreClusteringVertices &) = delete;
  NeighborhoodOfPreClusteringVertices &operator=(const NeighborhoodOfPreClusteringVertices &&) = delete;

 private:
  /**
 * Factory that create new clustering.
 */
  IClustFactoryPtr clustering_factory_;

 public:

  NeighborhoodOfPreClusteringVertices(IClustFactoryPtr clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph,
                                                        unsigned first_cluster_vertex,
                                                        unsigned second_cluster_vertex) const;
};

}
