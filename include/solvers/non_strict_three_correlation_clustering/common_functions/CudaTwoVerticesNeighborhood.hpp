#pragma once

#include <vector>

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace non_strict_3cc {
/**
 * GPU back end of the TwoVerticesNeighborhood family for NS3CC. Plain C++
 * header; the implementation is compiled by nvcc.
 *
 * For every ordered pair of distinct vertices (i, j) the graph is split by
 * their neighborhoods (NeighborSplitter::SplitGraphByTwoVertices); optionally
 * each split is driven to a local optimum (LocalSearch::ComputeLocalOptimum).
 * One CUDA block handles one pair, threads of the block split the vertices
 * between them. Pairs are processed in chunks, so memory does not grow with
 * the number of pairs.
 */
class CudaTwoVerticesNeighborhood {
 public:
  struct Result {
    /** Best clustering over all pairs. */
    IClustPtr clustering;
    /** Distance of the best clustering. */
    unsigned distance;
    /** Pair whose split produced the best clustering. */
    unsigned first_vertex;
    unsigned second_vertex;
    /**
     * Distance of the (locally optimized) split of every pair, indexed by
     * first * n + second, UINT_MAX on the diagonal. Filled only when
     * requested (n^2 values).
     */
    std::vector<unsigned> distances;
  };

  /**
   * @param graph source graph.
   * @param factory factory used to build the returned clustering.
   * @param with_local_search run the local search on every split.
   * @param keep_distances fill Result::distances.
   * @return best clustering. Ties between pairs are broken towards the
   * lexicographically smallest pair.
   */
  static Result Run(const IGraph &graph, const IClustFactoryPtr &factory,
                    bool with_local_search, bool keep_distances = false);

  /**
   * @return @code true, if at least one CUDA device is usable.
   */
  static bool IsAvailable();
};
}  // namespace non_strict_3cc
