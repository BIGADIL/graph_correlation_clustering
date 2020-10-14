#pragma once

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"
#include "../common_functions/NSplitterForNS2CC.hpp"

namespace ns2cc {

  /**
   * Neighborhood algorithm with local search for NS2CC.
   *
   * @warning use multithreading.
   */
  class N1LSAlgorithmForNS2CC {
   public:
    N1LSAlgorithmForNS2CC() = delete;
    N1LSAlgorithmForNS2CC(const N1LSAlgorithmForNS2CC &&) = delete;
    N1LSAlgorithmForNS2CC &operator=(const N1LSAlgorithmForNS2CC &) = delete;
    N1LSAlgorithmForNS2CC &operator=(const N1LSAlgorithmForNS2CC &&) = delete;

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
    NSplitterForNS2CC neighbor_splitter_;

   private:
    void BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                                unsigned threadId,
                                                IClustPtr &local_best_clustering) const;

   public:
    N1LSAlgorithmForNS2CC(unsigned num_threads,
                          const IClustFactoryPtr &clustering_factory);
    /**
     * Calc best clustering.
     * @param graph source graph.
     * @return best clustering.
     */
    [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
  };
}
