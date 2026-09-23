#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodCuda.hpp"

#include <utility>

#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/CudaTwoVerticesNeighborhood.hpp"

non_strict_3cc::TwoVerticesNeighborhoodCuda::TwoVerticesNeighborhoodCuda(IClustFactoryPtr clustering_factory)
    : clustering_factory_(std::move(clustering_factory)) {}

IClustPtr non_strict_3cc::TwoVerticesNeighborhoodCuda::getBestNeighborhoodClustering(const IGraph& graph) const {
  return getBestSolution(graph).clustering;
}

Solution non_strict_3cc::TwoVerticesNeighborhoodCuda::getBestSolution(const IGraph& graph) const {
  auto result = CudaTwoVerticesNeighborhood::Run(graph, clustering_factory_, false);
  return {result.distance, std::move(result.clustering)};
}
