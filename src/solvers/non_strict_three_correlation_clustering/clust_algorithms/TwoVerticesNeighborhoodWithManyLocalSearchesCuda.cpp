#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithManyLocalSearchesCuda.hpp"

#include <utility>

#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/CudaTwoVerticesNeighborhood.hpp"

non_strict_3cc::TwoVerticesNeighborhoodWithManyLocalSearchesCuda::TwoVerticesNeighborhoodWithManyLocalSearchesCuda(IClustFactoryPtr clustering_factory)
    : clustering_factory_(std::move(clustering_factory)) {}

IClustPtr non_strict_3cc::TwoVerticesNeighborhoodWithManyLocalSearchesCuda::getBestNeighborhoodClustering(
    const IGraph &graph) const {
  return CudaTwoVerticesNeighborhood::Run(graph, clustering_factory_, true)
      .clustering;
}
