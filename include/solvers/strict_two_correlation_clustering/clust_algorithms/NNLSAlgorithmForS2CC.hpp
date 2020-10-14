#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NSplitterFor2CC.hpp"
class NNLSAlgorithmForS2CC {
 public:
  NNLSAlgorithmForS2CC() = delete;
  NNLSAlgorithmForS2CC(const NNLSAlgorithmForS2CC &&) = delete;
  NNLSAlgorithmForS2CC &operator=(const NNLSAlgorithmForS2CC &) = delete;
  NNLSAlgorithmForS2CC &operator=(const NNLSAlgorithmForS2CC &&) = delete;

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
  NNLSAlgorithmForS2CC(unsigned num_threads,
                        const IClustFactoryPtr &clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
};



