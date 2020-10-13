#pragma once

#include "../../clustering/factories/IClusteringFactory.hpp"
#include "NSplitter.hpp"

class NAlgorithm {
 public:
  NAlgorithm() = delete;
  NAlgorithm(const NAlgorithm &&) = delete;
  NAlgorithm &operator=(const NAlgorithm &) = delete;
  NAlgorithm &operator=(const NAlgorithm &&) = delete;

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
  NAlgorithm(unsigned num_threads,
                const IClustFactoryPtr &clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
};


