#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodCuda.hpp"

#include <utility>

#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/CudaTwoVerticesNeighborhood.hpp"

non_strict_3cc::TwoVerticesNeighborhoodCuda::TwoVerticesNeighborhoodCuda(IClustFactoryPtr clustering_factory)
    : clustering_factory_(std::move(clustering_factory)) {}

IClustPtr non_strict_3cc::TwoVerticesNeighborhoodCuda::getBestNeighborhoodClustering(
    const IGraph &graph) const {
  return CudaTwoVerticesNeighborhood::Run(graph, clustering_factory_, false)
      .clustering;
}
