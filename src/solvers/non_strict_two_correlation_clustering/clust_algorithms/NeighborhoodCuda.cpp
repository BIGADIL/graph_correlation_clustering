#include "../../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodCuda.hpp"

#include <utility>

#include "../../../../include/solvers/non_strict_two_correlation_clustering/common_functions/CudaNeighborhood.hpp"

non_strict_2cc::NeighborhoodCuda::NeighborhoodCuda(IClustFactoryPtr clustering_factory)
    : clustering_factory_(std::move(clustering_factory)) {}

IClustPtr non_strict_2cc::NeighborhoodCuda::getBestNeighborhoodClustering(
    const IGraph &graph) const {
  return CudaNeighborhood::Run(graph, clustering_factory_, false).clustering;
}
