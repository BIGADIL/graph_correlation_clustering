#pragma once

#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NeighborSplitter.hpp"

namespace semi_supervised_2cc {

class NeighborhoodWithOneLocalSearch {
 public:
  NeighborhoodWithOneLocalSearch() = delete;
  NeighborhoodWithOneLocalSearch(const NeighborhoodWithOneLocalSearch &&) = delete;
  NeighborhoodWithOneLocalSearch &operator=(const NeighborhoodWithOneLocalSearch &) = delete;
  NeighborhoodWithOneLocalSearch &operator=(const NeighborhoodWithOneLocalSearch &&) = delete;

 private:
  /**
   * How many threads used by solver.
   */
  unsigned num_threads_;
  /**
  * Factory that create new clustering.
  */
  IClustFactoryPtr clustering_factory_;
  /**
   * Neighborhood splitter.
   */
  NeighborSplitter neighbor_splitter_;

 private:
  void BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                              unsigned threadId,
                                              IClustPtr &local_best_clustering,
                                              unsigned first_cluster_vertex,
                                              unsigned second_cluster_vertex) const;

 public:
  NeighborhoodWithOneLocalSearch(unsigned num_threads,
                                 const IClustFactoryPtr &clustering_factory);
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
