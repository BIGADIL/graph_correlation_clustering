#pragma once

#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NeighborSplitter.hpp"

namespace non_strict_3cc {

class NeighborhoodWith2LocalSearches {
 public:
  NeighborhoodWith2LocalSearches() = delete;
  NeighborhoodWith2LocalSearches(const NeighborhoodWith2LocalSearches &&) = delete;
  NeighborhoodWith2LocalSearches &operator=(const NeighborhoodWith2LocalSearches &) = delete;
  NeighborhoodWith2LocalSearches &operator=(const NeighborhoodWith2LocalSearches &&) = delete;

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
                                              IClustPtr &local_best_clustering) const;

 public:
  NeighborhoodWith2LocalSearches(unsigned num_threads,
                              const IClustFactoryPtr& clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
};

}
