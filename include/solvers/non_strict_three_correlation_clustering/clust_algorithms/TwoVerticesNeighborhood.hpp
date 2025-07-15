#pragma once

#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NeighborSplitter.hpp"

namespace non_strict_3cc {

class TwoVerticesNeighborhood {
 public:
  TwoVerticesNeighborhood() = delete;
  TwoVerticesNeighborhood(const TwoVerticesNeighborhood &&) = delete;
  TwoVerticesNeighborhood &operator=(const TwoVerticesNeighborhood &) = delete;
  TwoVerticesNeighborhood &operator=(const TwoVerticesNeighborhood &&) = delete;

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
  TwoVerticesNeighborhood(unsigned num_threads,
                              const IClustFactoryPtr& clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;

  [[nodiscard]] std::vector<Solution> getAllSolutions(const IGraph &graph) const;
};

}
