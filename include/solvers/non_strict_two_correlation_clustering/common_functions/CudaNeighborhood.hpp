#pragma once

#include <vector>

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace non_strict_2cc {
/**
 * GPU back end of the Neighborhood family for NS2CC. Plain C++ header; the
 * implementation is compiled by nvcc.
 *
 * For every vertex v the graph is split by the neighborhood of v
 * (NeighborSplitter::SplitGraphByVertex); optionally each split is driven to
 * a local optimum (LocalSearch::ComputeLocalOptimum). One CUDA block handles
 * one vertex, threads of the block split the vertices between them.
 */
class CudaNeighborhood {
 public:
  struct Result {
    /** Best clustering over all vertices. */
    IClustPtr clustering;
    /** Distance of the best clustering. */
    unsigned distance;
    /** Vertex whose split produced the best clustering. */
    unsigned vertex;
    /** Distance of the (locally optimized) split of every vertex. */
    std::vector<unsigned> distances;
  };

  /**
   * @param graph source graph.
   * @param factory factory used to build the returned clustering.
   * @param with_local_search run the local search on every split.
   * @return best clustering and per-vertex distances. Ties between vertices
   * are broken towards the smallest vertex index.
   */
  static Result Run(const IGraph& graph, const IClustFactoryPtr& factory, bool with_local_search);

  /**
   * @return @code true, if at least one CUDA device is usable.
   */
  static bool IsAvailable();
};
}  // namespace non_strict_2cc
