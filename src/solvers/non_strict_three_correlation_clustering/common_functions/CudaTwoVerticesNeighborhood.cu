#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/CudaTwoVerticesNeighborhood.hpp"

#include <algorithm>
#include <climits>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../../../include/common/CudaCommon.cuh"
#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/CudaLocalSearch.cuh"

namespace {

using gcc_cuda::DevArray;
using gcc_cuda::Label;
using non_strict_3cc::cuda::kNumLabels;

// Dynamic shared memory holds one label per vertex.
constexpr unsigned kMaxVertices = gcc_cuda::kSharedBudget;
constexpr char kName[] = "non_strict_3cc::CudaTwoVerticesNeighborhood";
// Scratch memory per chunk of pairs is kept under this budget.
constexpr size_t kChunkBudgetBytes = size_t{1} << 30;

/** Ordered pair number q in [0, n * (n - 1)) -> (first, second). */
__host__ __device__ void PairFromIndex(const unsigned long long q,
                                       const unsigned n,
                                       unsigned &first,
                                       unsigned &second) {
  first = static_cast<unsigned>(q / (n - 1));
  const unsigned r = static_cast<unsigned>(q % (n - 1));
  second = r < first ? r : r + 1;
}

/**
 * Block b handles pair first_pair + b: split the graph by the two vertices,
 * optionally run the local search, store labels and distance.
 */
template <bool kLocalSearch>
__global__ void TwoVerticesKernel(const uint8_t *__restrict__ adj,
                                  const unsigned n,
                                  const unsigned long long first_pair,
                                  Label *__restrict__ out_labels,
                                  int *__restrict__ gains_scratch,
                                  unsigned *__restrict__ out_dist) {
  extern __shared__ Label s_labels[];
  unsigned first, second;
  PairFromIndex(first_pair + blockIdx.x, n, first, second);

  non_strict_3cc::cuda::SplitByTwoVertices(adj, n, first, second, s_labels);
  unsigned distance;
  if constexpr (kLocalSearch) {
    int *gains = gains_scratch + static_cast<size_t>(blockIdx.x) * n * kNumLabels;
    distance = non_strict_3cc::cuda::ComputeGains(adj, n, s_labels, gains);
    distance =
        non_strict_3cc::cuda::LocalSearchBlock(adj, n, s_labels, gains, distance);
  } else {
    distance = non_strict_3cc::cuda::ComputeDistance(adj, n, s_labels);
  }

  Label *labels = out_labels + static_cast<size_t>(blockIdx.x) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    labels[i] = s_labels[i];
  }
  if (threadIdx.x == 0) {
    out_dist[blockIdx.x] = distance;
  }
}

ClusterLabels ToClusterLabel(const Label label) {
  switch (label) {
    case 0:
      return FIRST_CLUSTER;
    case 1:
      return SECOND_CLUSTER;
    default:
      return THIRD_CLUSTER;
  }
}

}  // namespace

bool non_strict_3cc::CudaTwoVerticesNeighborhood::IsAvailable() {
  return gcc_cuda::HasCudaDevice();
}

non_strict_3cc::CudaTwoVerticesNeighborhood::Result
non_strict_3cc::CudaTwoVerticesNeighborhood::Run(
    const IGraph &graph, const IClustFactoryPtr &factory,
    const bool with_local_search, const bool keep_distances) {
  const unsigned n = graph.Size();
  gcc_cuda::CheckGraphSize(n, kMaxVertices, kName);
  if (n < 2) {
    throw std::invalid_argument(std::string(kName) +
                                " needs at least 2 vertices");
  }
  const unsigned threads = gcc_cuda::ThreadsForSize(n);
  const size_t labels_bytes = static_cast<size_t>(n) * sizeof(Label);
  const unsigned long long num_pairs =
      static_cast<unsigned long long>(n) * (n - 1);

  const size_t per_pair =
      static_cast<size_t>(n) *
      (sizeof(Label) + (with_local_search ? kNumLabels * sizeof(int) : 0));
  const unsigned chunk = static_cast<unsigned>(std::min<unsigned long long>(
      std::clamp<size_t>(kChunkBudgetBytes / per_pair, 1, 65535), num_pairs));

  DevArray<uint8_t> d_adj(static_cast<size_t>(n) * n);
  d_adj.Upload(gcc_cuda::FlattenGraph(graph));
  DevArray<Label> d_labels(static_cast<size_t>(chunk) * n);
  DevArray<int> d_gains(with_local_search
                            ? static_cast<size_t>(chunk) * n * kNumLabels
                            : 0);
  DevArray<unsigned> d_dist(chunk);
  DevArray<Label> d_best(n);

  Result result;
  result.distance = UINT_MAX;
  result.first_vertex = result.second_vertex = UINT_MAX;
  if (keep_distances) {
    result.distances.assign(static_cast<size_t>(n) * n, UINT_MAX);
  }

  std::vector<unsigned> h_dist;
  for (unsigned long long first = 0; first < num_pairs; first += chunk) {
    const unsigned count =
        static_cast<unsigned>(std::min<unsigned long long>(chunk, num_pairs - first));
    if (with_local_search) {
      TwoVerticesKernel<true><<<count, threads, labels_bytes>>>(
          d_adj.Get(), n, first, d_labels.Get(), d_gains.Get(), d_dist.Get());
    } else {
      TwoVerticesKernel<false><<<count, threads, labels_bytes>>>(
          d_adj.Get(), n, first, d_labels.Get(), nullptr, d_dist.Get());
    }
    GCC_CUDA_CHECK(cudaGetLastError());
    d_dist.Download(h_dist);

    unsigned best_slot = UINT_MAX;
    for (unsigned s = 0; s < count; ++s) {
      if (keep_distances) {
        unsigned i, j;
        PairFromIndex(first + s, n, i, j);
        result.distances[static_cast<size_t>(i) * n + j] = h_dist[s];
      }
      if (h_dist[s] < result.distance) {
        result.distance = h_dist[s];
        best_slot = s;
      }
    }
    if (best_slot != UINT_MAX) {
      PairFromIndex(first + best_slot, n, result.first_vertex,
                    result.second_vertex);
      GCC_CUDA_CHECK(cudaMemcpy(
          d_best.Get(), d_labels.Get() + static_cast<size_t>(best_slot) * n,
          labels_bytes, cudaMemcpyDeviceToDevice));
    }
  }

  std::vector<Label> h_best;
  d_best.Download(h_best);
  result.clustering = factory->CreateClustering(n);
  for (unsigned i = 0; i < n; ++i) {
    result.clustering->SetupLabelForVertex(i, ToClusterLabel(h_best[i]));
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
