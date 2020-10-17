#pragma once

#include <vector>

#include "../../../clustering/IClustering.hpp"

namespace set_semi_supervised_2cc {

class LocalSearch {
 private:
  /**
   * Compute local improvement of tossing vertex from one cluster to another.
   *
   * @param graph source graph.
   * @param cur_clustering current clustering.
   * @param vertex vertex to toss.
   * @return local improvement of tossing vertex from one cluster to another.
   */
  static int ComputeLocalImprovement(const IGraph &graph,
                                     const IClustPtr &cur_clustering,
                                     unsigned vertex);

  static bool IsVertexInSet(const unsigned vertex, const std::vector<unsigned> &vertices_set);

 public:
  /**
   * Compute local optimal solution for the source graph.
   *
   * @param graph source graph.
   * @param cur_clustering init clustering.
   * @return local optimal clustering.
   */
  static IClustPtr ComputeLocalOptimum(const IGraph &graph,
                                       const IClustPtr &cur_clustering,
                                       const std::vector<unsigned> &first_cluster_vertices,
                                       const std::vector<unsigned> &second_cluster_vertices);
};

}
