#pragma once

#include <memory>
#include <mutex>

#include "../../clustering/IClustering.hpp"
#include "../../clustering/factories/IClusteringFactory.hpp"
#include "NSplitterForNS2CC.hpp"
#include "LSAlgorithmForNS2CC.hpp"

namespace ns2cc {

  /**
   * Neighborhood algorithm with local search for NS2CC.
   *
   * @warning use multithreading.
   */
  class NNLSAlgorithmForNS2CC {
   public:
    NNLSAlgorithmForNS2CC() = delete;
    NNLSAlgorithmForNS2CC(const NNLSAlgorithmForNS2CC &&) = delete;
    NNLSAlgorithmForNS2CC &operator=(const NNLSAlgorithmForNS2CC &) = delete;
    NNLSAlgorithmForNS2CC &operator=(const NNLSAlgorithmForNS2CC &&) = delete;

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
    NNLSAlgorithmForNS2CC(unsigned num_threads,
                          const IClustFactoryPtr &clustering_factory);
    /**
     * Calc best clustering.
     * @param graph source graph.
     * @return best clustering.
     */
    [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
  };
}