#pragma once

#include <vector>
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NeighborSplitter.hpp"

namespace non_strict_3cc {

class TwoVerticesNeighborhoodWithManyLocalSearches {
 public:
  TwoVerticesNeighborhoodWithManyLocalSearches() = delete;
  TwoVerticesNeighborhoodWithManyLocalSearches(const TwoVerticesNeighborhoodWithManyLocalSearches &&) = delete;
  TwoVerticesNeighborhoodWithManyLocalSearches &operator=(const TwoVerticesNeighborhoodWithManyLocalSearches &) = delete;
  TwoVerticesNeighborhoodWithManyLocalSearches &operator=(const TwoVerticesNeighborhoodWithManyLocalSearches &&) = delete;

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
                                              std::vector<Solution> &local_thread_buffer) const;

 public:
  TwoVerticesNeighborhoodWithManyLocalSearches(unsigned num_threads,
                                               const IClustFactoryPtr &clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;

  [[nodiscard]] std::vector<Solution> getAllSolutions(const IGraph &graph) const;
};

}
