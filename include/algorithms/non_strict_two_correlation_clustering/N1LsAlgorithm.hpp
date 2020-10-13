#pragma once

#include "../../graphs/IGraph.hpp"
#include "../../clustering/IClustering.hpp"
#include "NSplitter.hpp"
/**
 * Neighborhood algorithm with local search for NS2CC.
 *
 * @warning use multithreading.
 */
class N1LSAlgorithm {
 public:
  N1LSAlgorithm() = delete;
  N1LSAlgorithm(const N1LSAlgorithm &&) = delete;
  N1LSAlgorithm &operator=(const N1LSAlgorithm &) = delete;
  N1LSAlgorithm &operator=(const N1LSAlgorithm &&) = delete;

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
  N1LSAlgorithm(unsigned num_threads,
                const IClustFactoryPtr &clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
};



