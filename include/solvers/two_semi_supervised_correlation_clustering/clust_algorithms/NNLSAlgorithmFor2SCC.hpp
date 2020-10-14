#pragma once

#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NSplitterFor2SCC.hpp"
class NNLSAlgorithmFor2SCC {
 public:
  NNLSAlgorithmFor2SCC() = delete;
  NNLSAlgorithmFor2SCC(const NNLSAlgorithmFor2SCC &&) = delete;
  NNLSAlgorithmFor2SCC &operator=(const NNLSAlgorithmFor2SCC &) = delete;
  NNLSAlgorithmFor2SCC &operator=(const NNLSAlgorithmFor2SCC &&) = delete;

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
  NSplitterFor2SCC neighbor_splitter_;

 private:
  void BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                              unsigned threadId,
                                              IClustPtr &local_best_clustering,
                                              unsigned first_cluster_vertex,
                                              unsigned second_cluster_vertex) const;

 public:
  NNLSAlgorithmFor2SCC(unsigned num_threads,
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



