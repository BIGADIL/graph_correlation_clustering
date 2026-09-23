#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace non_strict_3cc {
/**
 * CUDA implementation of TwoVerticesNeighborhood for NS3CC.
 *
 * For every ordered pair of vertices the graph is split by their
 * neighborhoods on the GPU and the split with the smallest distance is
 * returned.
 * Available only when the project is built with CUDA (GCC_HAS_CUDA).
 */
class TwoVerticesNeighborhoodCuda {
  IClustFactoryPtr clustering_factory_;

 public:
  TwoVerticesNeighborhoodCuda() = delete;

  TwoVerticesNeighborhoodCuda(const TwoVerticesNeighborhoodCuda&&) = delete;

  TwoVerticesNeighborhoodCuda& operator=(const TwoVerticesNeighborhoodCuda&) = delete;

  TwoVerticesNeighborhoodCuda& operator=(const TwoVerticesNeighborhoodCuda&&) = delete;

  explicit TwoVerticesNeighborhoodCuda(IClustFactoryPtr clustering_factory);

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
}  // namespace non_strict_3cc
