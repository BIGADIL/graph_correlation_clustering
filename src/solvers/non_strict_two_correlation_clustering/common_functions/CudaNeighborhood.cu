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
// Scratch memory per chunk of vertices (GEMM output reused as improvements,
// plus labels) is kept under this budget.
constexpr size_t kChunkBudgetBytes = size_t{1} << 30;

/**
 * Block b handles vertex first_vertex + b: split the graph by its
 * neighborhood, take the improvements from the GEMM output column
 * (overwritten in place), optionally run the local search, store labels and
 * distance.
 *
 * The split of vertex v has the +-1 label vector B[:, v] + e_v, so the GEMM
 * B * (B + I) yields (B s) for every split at once.
 */
template <bool kLocalSearch>
__global__ void NeighborhoodKernel(const uint8_t *__restrict__ adj,
                                   const unsigned n, const unsigned n_pad,
                                   const unsigned first_vertex,
                                   int *__restrict__ gmat,
                                   Label *__restrict__ out_labels,
                                   unsigned *__restrict__ out_dist) {
  extern __shared__ Label s_labels[];
  const unsigned v = first_vertex + blockIdx.x;
  int *impr = gmat + static_cast<size_t>(blockIdx.x) * n_pad;

  non_strict_2cc::cuda::SplitByVertex(adj, n, v, s_labels);
  unsigned distance =
      non_strict_2cc::cuda::ImprovementsFromGemm(n, s_labels, impr, impr);
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
  const unsigned n_pad = gcc_cuda::PadTo4(n);
  const unsigned threads = gcc_cuda::ThreadsForSize(n);
  const size_t labels_bytes = static_cast<size_t>(n) * sizeof(Label);

  const size_t per_vertex =
      static_cast<size_t>(n_pad) * sizeof(int) + static_cast<size_t>(n);
  const unsigned chunk = static_cast<unsigned>(
      std::clamp<size_t>(kChunkBudgetBytes / per_vertex, 1, n));

  DevArray<uint8_t> d_adj(static_cast<size_t>(n) * n);
  d_adj.Upload(gcc_cuda::FlattenGraph(graph));
  // B: +-1 adjacency with zero diagonal; S = B + I: the +-1 labels of all
  // neighborhood splits, one per column.
  const size_t padded_cells = static_cast<size_t>(n_pad) * n_pad;
  DevArray<int8_t> d_bmat(padded_cells), d_smat(padded_cells);
  gcc_cuda::BuildPaddedMatrix(d_adj.Get(), n, n_pad, 1, -1, 0, d_bmat.Get());
  gcc_cuda::BuildPaddedMatrix(d_adj.Get(), n, n_pad, 1, -1, 1, d_smat.Get());
  DevArray<int> d_gmat(static_cast<size_t>(chunk) * n_pad);
  gcc_cuda::CublasHandle cublas;

  DevArray<Label> d_labels(static_cast<size_t>(chunk) * n);
  DevArray<unsigned> d_dist(chunk);
  DevArray<Label> d_best(n);

  Result result;
  result.distance = UINT_MAX;
  result.vertex = UINT_MAX;
  result.distances.resize(n);

  std::vector<unsigned> h_dist;
  for (unsigned first = 0; first < n; first += chunk) {
    const unsigned count = std::min(chunk, n - first);
    gcc_cuda::GemmInt8(cublas, n_pad, count, n_pad, d_bmat.Get(),
                       d_smat.Get() + static_cast<size_t>(first) * n_pad,
                       d_gmat.Get());
    if (with_local_search) {
      NeighborhoodKernel<true><<<count, threads, labels_bytes>>>(
          d_adj.Get(), n, n_pad, first, d_gmat.Get(), d_labels.Get(),
          d_dist.Get());
    } else {
      NeighborhoodKernel<false><<<count, threads, labels_bytes>>>(
          d_adj.Get(), n, n_pad, first, d_gmat.Get(), d_labels.Get(),
          d_dist.Get());
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
  return result;
}
