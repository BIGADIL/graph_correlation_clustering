#include "../../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithManyLocalSearchesCuda.hpp"

#include <utility>

#include "../../../../include/solvers/non_strict_two_correlation_clustering/common_functions/CudaNeighborhood.hpp"

non_strict_2cc::NeighborhoodWithManyLocalSearchesCuda::NeighborhoodWithManyLocalSearchesCuda(
    IClustFactoryPtr clustering_factory)
    : clustering_factory_(std::move(clustering_factory)) {}

IClustPtr non_strict_2cc::NeighborhoodWithManyLocalSearchesCuda::getBestNeighborhoodClustering(
    const IGraph& graph) const {
  return getBestSolution(graph).clustering;
}

Solution non_strict_2cc::NeighborhoodWithManyLocalSearchesCuda::getBestSolution(const IGraph& graph) const {
  auto result = CudaNeighborhood::Run(graph, clustering_factory_, true);
  return {result.distance, std::move(result.clustering)};
}
