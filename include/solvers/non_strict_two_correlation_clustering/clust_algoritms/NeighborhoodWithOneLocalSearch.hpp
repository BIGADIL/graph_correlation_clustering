#pragma once

#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"
#include "../common_functions/NeighborSplitter.hpp"

namespace non_strict_2cc {

  /**
   * Neighborhood algorithm with local search for NS2CC.
   *
   * @warning use multithreading.
   */
  class NeighborhoodWithOneLocalSearch {
   public:
    NeighborhoodWithOneLocalSearch() = delete;
    NeighborhoodWithOneLocalSearch(const NeighborhoodWithOneLocalSearch &&) = delete;
    NeighborhoodWithOneLocalSearch &operator=(const NeighborhoodWithOneLocalSearch &) = delete;
    NeighborhoodWithOneLocalSearch &operator=(const NeighborhoodWithOneLocalSearch &&) = delete;

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
    NeighborSplitter neighbor_splitter_;

   private:
    void BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                                unsigned threadId,
                                                std::vector<Solution> &local_thread_buffer) const;

   public:
    NeighborhoodWithOneLocalSearch(unsigned num_threads,
                                   const IClustFactoryPtr &clustering_factory);
    /**
     * Calc best clustering.
     * @param graph source graph.
     * @return best clustering.
     */
    [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph &graph) const;

    [[nodiscard]] std::vector<Solution> getAllSolutions(const IGraph &graph) const;
  };
}
