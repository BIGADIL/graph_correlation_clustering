#pragma once

#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NSplitterFor2CC.hpp"

class N1LSAlgorithmForS2CC {
 public:
  N1LSAlgorithmForS2CC() = delete;
  N1LSAlgorithmForS2CC(const N1LSAlgorithmForS2CC &&) = delete;
  N1LSAlgorithmForS2CC &operator=(const N1LSAlgorithmForS2CC &) = delete;
  N1LSAlgorithmForS2CC &operator=(const N1LSAlgorithmForS2CC &&) = delete;

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
                                              IClustPtr &local_best_clustering,
                                              unsigned &vertex,
                                              unsigned &opposite_vertex) const;

 public:
  N1LSAlgorithmForS2CC(unsigned num_threads,
                        const IClustFactoryPtr &clustering_factory);
  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
};



