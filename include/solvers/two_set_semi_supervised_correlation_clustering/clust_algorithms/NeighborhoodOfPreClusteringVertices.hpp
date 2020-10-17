#pragma once

#include <vector>

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace set_semi_supervised_2cc {

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

 private:
  static bool IsJoinedWithSet(const std::vector<unsigned> &set, const unsigned vertex, const IGraph &graph);
  static bool IsVertexInSet(const unsigned vertex, const std::vector<unsigned> &vertices_set);

 public:
  NeighborhoodOfPreClusteringVertices(IClustFactoryPtr clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph,
                                                        const std::vector<unsigned> &first_cluster_vertices,
                                                        const std::vector<unsigned> &second_cluster_vertices) const;
};

}

