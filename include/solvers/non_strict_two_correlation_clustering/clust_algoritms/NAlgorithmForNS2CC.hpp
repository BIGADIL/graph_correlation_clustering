#pragma once

#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../common_functions/NSplitterForNS2CC.hpp"

namespace ns2cc {

  class NAlgorithmForNS2CC {
   public:
    NAlgorithmForNS2CC() = delete;
    NAlgorithmForNS2CC(const NAlgorithmForNS2CC &&) = delete;
    NAlgorithmForNS2CC &operator=(const NAlgorithmForNS2CC &) = delete;
    NAlgorithmForNS2CC &operator=(const NAlgorithmForNS2CC &&) = delete;

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
    NAlgorithmForNS2CC(unsigned num_threads,
                       const IClustFactoryPtr &clustering_factory);
    /**
     * Calc best clustering.
     * @param graph source graph.
     * @return best clustering.
     */
    [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;
  };
}


