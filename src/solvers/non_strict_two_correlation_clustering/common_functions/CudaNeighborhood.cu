#include "../../../../include/solvers/non_strict_two_correlation_clustering/common_functions/CudaNeighborhood.hpp"

#include <algorithm>
#include <climits>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../../../include/common/CudaCommon.cuh"
#include "../../../../include/solvers/non_strict_two_correlation_clustering/common_functions/CudaLocalSearch.cuh"

namespace {

using gcc_cuda::DevArray;
using gcc_cuda::Label;

// Dynamic shared memory holds one label per vertex.
constexpr unsigned kMaxVertices = gcc_cuda::kSharedBudget;
constexpr char kName[] = "non_strict_2cc::CudaNeighborhood";
// Scratch memory per chunk of vertices: (sizeof(int) + sizeof(Label)) * n
// bytes per vertex, kept under this budget.
constexpr size_t kChunkBudgetBytes = size_t{1} << 30;

/**
 * Block b handles vertex first_vertex + b: split the graph by its
 * neighborhood, optionally run the local search, store labels and distance.
 */
template <bool kLocalSearch>
__global__ void NeighborhoodKernel(const uint8_t *__restrict__ adj,
                                   const unsigned n,
                                   const unsigned first_vertex,
                                   Label *__restrict__ out_labels,
                                   int *__restrict__ impr_scratch,
                                   unsigned *__restrict__ out_dist) {
  extern __shared__ Label s_labels[];
  const unsigned v = first_vertex + blockIdx.x;
  int *impr = impr_scratch + static_cast<size_t>(blockIdx.x) * n;

  non_strict_2cc::cuda::SplitByVertex(adj, n, v, s_labels);
  unsigned distance =
      non_strict_2cc::cuda::ComputeImprovements(adj, n, s_labels, impr);
  if constexpr (kLocalSearch) {
    distance =
        non_strict_2cc::cuda::LocalSearchBlock(adj, n, s_labels, impr, distance);
  }

  Label *labels = out_labels + static_cast<size_t>(blockIdx.x) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    labels[i] = s_labels[i];
  }
  if (threadIdx.x == 0) {
    out_dist[blockIdx.x] = distance;
  }
}

}  // namespace

bool non_strict_2cc::CudaNeighborhood::IsAvailable() {
  return gcc_cuda::HasCudaDevice();
}

non_strict_2cc::CudaNeighborhood::Result non_strict_2cc::CudaNeighborhood::Run(
    const IGraph &graph, const IClustFactoryPtr &factory,
    const bool with_local_search) {
  const unsigned n = graph.Size();
  gcc_cuda::CheckGraphSize(n, kMaxVertices, kName);
  const unsigned threads = gcc_cuda::ThreadsForSize(n);
  const size_t labels_bytes = static_cast<size_t>(n) * sizeof(Label);

  const size_t per_vertex = static_cast<size_t>(n) * (sizeof(int) + sizeof(Label));
  const unsigned chunk = static_cast<unsigned>(std::clamp<size_t>(
      kChunkBudgetBytes / per_vertex, 1, n));

  DevArray<uint8_t> d_adj(static_cast<size_t>(n) * n);
  d_adj.Upload(gcc_cuda::FlattenGraph(graph));
  DevArray<Label> d_labels(static_cast<size_t>(chunk) * n);
  DevArray<int> d_impr(static_cast<size_t>(chunk) * n);
  DevArray<unsigned> d_dist(chunk);
  DevArray<Label> d_best(n);

  Result result;
  result.distance = UINT_MAX;
  result.vertex = UINT_MAX;
  result.distances.resize(n);

  std::vector<unsigned> h_dist;
  for (unsigned first = 0; first < n; first += chunk) {
    const unsigned count = std::min(chunk, n - first);
    if (with_local_search) {
      NeighborhoodKernel<true><<<count, threads, labels_bytes>>>(
          d_adj.Get(), n, first, d_labels.Get(), d_impr.Get(), d_dist.Get());
    } else {
      NeighborhoodKernel<false><<<count, threads, labels_bytes>>>(
          d_adj.Get(), n, first, d_labels.Get(), d_impr.Get(), d_dist.Get());
    }
    GCC_CUDA_CHECK(cudaGetLastError());
    d_dist.Download(h_dist);

    unsigned best_slot = UINT_MAX;
    for (unsigned s = 0; s < count; ++s) {
      result.distances[first + s] = h_dist[s];
      if (h_dist[s] < result.distance) {
        result.distance = h_dist[s];
        result.vertex = first + s;
        best_slot = s;
      }
    }
    if (best_slot != UINT_MAX) {
      GCC_CUDA_CHECK(cudaMemcpy(
          d_best.Get(), d_labels.Get() + static_cast<size_t>(best_slot) * n,
          labels_bytes, cudaMemcpyDeviceToDevice));
    }
  }

  std::vector<Label> h_best;
  d_best.Download(h_best);
  result.clustering = factory->CreateClustering(n);
  for (unsigned i = 0; i < n; ++i) {
    result.clustering->SetupLabelForVertex(
        i, h_best[i] == 0 ? FIRST_CLUSTER : SECOND_CLUSTER);
  }
  const unsigned host_dist = result.clustering->GetDistanceToGraph(graph);
  if (host_dist != result.distance) {
    throw std::logic_error(std::string(kName) + ": GPU distance " +
                           std::to_string(result.distance) +
                           " differs from host distance " +
                           std::to_string(host_dist));
  }
  return result;
}
