#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace non_strict_2cc {
/**
 * CUDA implementation of Neighborhood for NS2CC.
 *
 * For every vertex the graph is split by its neighborhood on the GPU and the
 * split with the smallest distance is returned.
 * Available only when the project is built with CUDA (GCC_HAS_CUDA).
 */
class NeighborhoodCuda {
  IClustFactoryPtr clustering_factory_;

 public:
  NeighborhoodCuda() = delete;

  NeighborhoodCuda(const NeighborhoodCuda &&) = delete;

  NeighborhoodCuda &operator=(const NeighborhoodCuda &) = delete;

  NeighborhoodCuda &operator=(const NeighborhoodCuda &&) = delete;

  explicit NeighborhoodCuda(IClustFactoryPtr clustering_factory);

  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(
      const IGraph &graph) const;
};
}  // namespace non_strict_2cc
