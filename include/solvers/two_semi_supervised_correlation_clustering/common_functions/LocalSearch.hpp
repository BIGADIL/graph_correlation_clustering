#pragma once

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"

namespace semi_supervised_2cc {

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
                                       unsigned first_cluster_vertex,
                                       unsigned second_cluster_vertex);
};

}
