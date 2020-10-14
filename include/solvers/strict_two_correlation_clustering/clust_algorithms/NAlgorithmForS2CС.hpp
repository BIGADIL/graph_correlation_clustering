#pragma once

#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NSplitterFor2CC.hpp"

class NAlgorithmForS2CC {
 public:
  NAlgorithmForS2CC() = delete;
  NAlgorithmForS2CC(const NAlgorithmForS2CC &&) = delete;
  NAlgorithmForS2CC &operator=(const NAlgorithmForS2CC &) = delete;
  NAlgorithmForS2CC &operator=(const NAlgorithmForS2CC &&) = delete;

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
  NSplitterForS2CC neighbor_splitter_;

 private:
  void BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                              unsigned threadId,
                                              IClustPtr &local_best_clustering) const;

 public:
  NAlgorithmForS2CC(unsigned num_threads,
                    const IClustFactoryPtr &clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
};



