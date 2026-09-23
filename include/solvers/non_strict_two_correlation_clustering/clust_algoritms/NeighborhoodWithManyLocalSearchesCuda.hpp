#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace non_strict_2cc {
/**
 * CUDA implementation of NeighborhoodWithManyLocalSearches for NS2CC.
 *
 * For every vertex the graph is split by its neighborhood and driven to a
 * local optimum on the GPU; the local optimum with the smallest distance is
 * returned.
 * Available only when the project is built with CUDA (GCC_HAS_CUDA).
 */
class NeighborhoodWithManyLocalSearchesCuda {
  IClustFactoryPtr clustering_factory_;

 public:
  NeighborhoodWithManyLocalSearchesCuda() = delete;

  NeighborhoodWithManyLocalSearchesCuda(const NeighborhoodWithManyLocalSearchesCuda&&) = delete;

  NeighborhoodWithManyLocalSearchesCuda& operator=(const NeighborhoodWithManyLocalSearchesCuda&) = delete;

  NeighborhoodWithManyLocalSearchesCuda& operator=(const NeighborhoodWithManyLocalSearchesCuda&&) = delete;

  explicit NeighborhoodWithManyLocalSearchesCuda(IClustFactoryPtr clustering_factory);

  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(const IGraph& graph) const;

  /**
   * Calc best clustering together with its distance (computed on the GPU, no
   * extra O(n^2) pass on the host).
   * @param graph source graph.
   * @return best solution.
   */
  [[nodiscard]] Solution getBestSolution(const IGraph& graph) const;
};
}  // namespace non_strict_2cc
