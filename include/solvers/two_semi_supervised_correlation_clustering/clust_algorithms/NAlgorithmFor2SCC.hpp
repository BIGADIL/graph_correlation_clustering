#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NSplitterFor2SCC.hpp"

class NAlgorithmFor2SCC {
 public:
  NAlgorithmFor2SCC() = delete;
  NAlgorithmFor2SCC(const NAlgorithmFor2SCC &&) = delete;
  NAlgorithmFor2SCC &operator=(const NAlgorithmFor2SCC &) = delete;
  NAlgorithmFor2SCC &operator=(const NAlgorithmFor2SCC &&) = delete;

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
  NAlgorithmFor2SCC(unsigned num_threads,
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



