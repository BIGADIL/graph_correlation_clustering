#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NeighborSplitter.hpp"

namespace strict_2cc {

class NeighborhoodWithManyLocalSearches {
 public:
  NeighborhoodWithManyLocalSearches() = delete;
  NeighborhoodWithManyLocalSearches(const NeighborhoodWithManyLocalSearches &&) = delete;
  NeighborhoodWithManyLocalSearches &operator=(const NeighborhoodWithManyLocalSearches &) = delete;
  NeighborhoodWithManyLocalSearches &operator=(const NeighborhoodWithManyLocalSearches &&) = delete;

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
  NeighborhoodWithManyLocalSearches(unsigned num_threads,
                                    const IClustFactoryPtr &clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
};

}
