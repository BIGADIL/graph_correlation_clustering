#pragma once

#include <memory>
#include <mutex>

#include "../../clustering/IClustering.hpp"
#include "../../clustering/factories/IClusteringFactory.hpp"
#include "NSplitter.hpp"
#include "LSAlgorithm.hpp"

/**
 * Neighborhood algorithm with local search for NS2CC.
 *
 * @warning use multithreading.
 */
class NNLSAlgorithm {
 public:
  NNLSAlgorithm() = delete;
  NNLSAlgorithm(const NNLSAlgorithm &&) = delete;
  NNLSAlgorithm &operator=(const NNLSAlgorithm &) = delete;
  NNLSAlgorithm &operator=(const NNLSAlgorithm &&) = delete;

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
  NSplitter neighbor_splitter_;

 private:
  void BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                              unsigned threadId,
                                              IClustPtr &local_best_clustering) const;

 public:
  NNLSAlgorithm(unsigned num_threads,
                const IClustFactoryPtr &clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
};
