#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithManyLocalSearchesCuda.hpp"

#include <utility>

#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/CudaTwoVerticesNeighborhood.hpp"

non_strict_3cc::TwoVerticesNeighborhoodWithManyLocalSearchesCuda::TwoVerticesNeighborhoodWithManyLocalSearchesCuda(
    IClustFactoryPtr clustering_factory)
    : clustering_factory_(std::move(clustering_factory)) {}

IClustPtr non_strict_3cc::TwoVerticesNeighborhoodWithManyLocalSearchesCuda::getBestNeighborhoodClustering(
    const IGraph& graph) const {
  return getBestSolution(graph).clustering;
}

Solution non_strict_3cc::TwoVerticesNeighborhoodWithManyLocalSearchesCuda::getBestSolution(const IGraph& graph) const {
  auto result = CudaTwoVerticesNeighborhood::Run(graph, clustering_factory_, true);
  return {result.distance, std::move(result.clustering)};
}
