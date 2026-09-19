#pragma once

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace non_strict_3cc {
/**
 * CUDA implementation of TwoVerticesNeighborhoodWithManyLocalSearches for NS3CC.
 *
 * For every ordered pair of vertices the graph is split by their
 * neighborhoods and driven to a local optimum on the GPU; the local optimum
 * with the smallest distance is returned.
 * Available only when the project is built with CUDA (GCC_HAS_CUDA).
 */
class TwoVerticesNeighborhoodWithManyLocalSearchesCuda {
  IClustFactoryPtr clustering_factory_;

 public:
  TwoVerticesNeighborhoodWithManyLocalSearchesCuda() = delete;

  TwoVerticesNeighborhoodWithManyLocalSearchesCuda(const TwoVerticesNeighborhoodWithManyLocalSearchesCuda &&) = delete;

  TwoVerticesNeighborhoodWithManyLocalSearchesCuda &operator=(const TwoVerticesNeighborhoodWithManyLocalSearchesCuda &) = delete;

  TwoVerticesNeighborhoodWithManyLocalSearchesCuda &operator=(const TwoVerticesNeighborhoodWithManyLocalSearchesCuda &&) = delete;

  explicit TwoVerticesNeighborhoodWithManyLocalSearchesCuda(IClustFactoryPtr clustering_factory);

  /**
   * Calc best clustering.
   * @param graph source graph.
   * @return best clustering.
   */
  [[nodiscard]] IClustPtr getBestNeighborhoodClustering(
      const IGraph &graph) const;
};
}  // namespace non_strict_3cc
