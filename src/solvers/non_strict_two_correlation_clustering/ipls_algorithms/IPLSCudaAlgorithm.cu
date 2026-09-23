#include "../../../../include/solvers/non_strict_two_correlation_clustering/ipls_algorithms/IPLSCudaAlgorithm.hpp"

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
using gcc_cuda::Rng;
using non_strict_2cc::cuda::ComputeImprovements;
using non_strict_2cc::cuda::ImprovementsFromGemm;
using non_strict_2cc::cuda::LocalSearchBlock;

// Dynamic shared memory holds one label per vertex.
constexpr unsigned kMaxVertices = gcc_cuda::kSharedBudget;
constexpr char kName[] = "non_strict_2cc::IPLSCudaAlgorithm";

// ---------------------------------------------------------------------------
// Kernels. One block per individual; blockIdx.x is the slot in the population.
//
// The O(n^2) computation of the improvements after the initial labeling and
// after every perturbation is done by one int8 GEMM for the whole population:
// the kernels only write the +-1 label columns, and FinishFromGemmKernel turns
// the GEMM output into improvements and distances.
// ---------------------------------------------------------------------------

__global__ void InitPopulationKernel(const unsigned n, const unsigned n_pad,
                                     Label *__restrict__ pop_labels,
                                     int8_t *__restrict__ smat,
                                     Rng *__restrict__ rng) {
  const unsigned slot = blockIdx.x;
  const unsigned rng_id = slot * blockDim.x + threadIdx.x;
  Rng state = rng[rng_id];

  Label *labels = pop_labels + static_cast<size_t>(slot) * n;
  int8_t *s_column = smat + static_cast<size_t>(slot) * n_pad;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    const Label label = static_cast<Label>(curand(&state) & 1u);
    labels[i] = label;
    s_column[i] = label == 0 ? 1 : -1;
  }
  rng[rng_id] = state;
}

/** Improvements and distance of every individual from the GEMM output. */
__global__ void FinishFromGemmKernel(const unsigned n, const unsigned n_pad,
                                     const Label *__restrict__ pop_labels,
                                     const int *__restrict__ gmat,
                                     int *__restrict__ pop_impr,
                                     unsigned *__restrict__ pop_dist) {
  const unsigned slot = blockIdx.x;
  const unsigned distance = ImprovementsFromGemm(
      n, pop_labels + static_cast<size_t>(slot) * n,
      gmat + static_cast<size_t>(slot) * n_pad,
      pop_impr + static_cast<size_t>(slot) * n);
  if (threadIdx.x == 0) {
    pop_dist[slot] = distance;
  }
}

/**
 * Tournament selection from the population, then local search.
 * Writes the local optimum into ls_* at the same slot.
 */
__global__ void SelectAndLocalSearchKernel(
    const uint8_t *__restrict__ adj, const unsigned n,
    const unsigned population_size, const unsigned tournament_size,
    const Label *__restrict__ pop_labels, const int *__restrict__ pop_impr,
    const unsigned *__restrict__ pop_dist, Label *__restrict__ ls_labels,
    int *__restrict__ ls_impr, unsigned *__restrict__ ls_dist,
    Rng *__restrict__ rng) {
  extern __shared__ Label s_labels[];
  __shared__ unsigned s_parent;
  const unsigned slot = blockIdx.x;

  if (threadIdx.x == 0) {
    const unsigned rng_id = slot * blockDim.x;
    Rng state = rng[rng_id];
    s_parent = gcc_cuda::Tournament(pop_dist, population_size,
                                    tournament_size, state);
    rng[rng_id] = state;
  }
  __syncthreads();

  const unsigned parent = s_parent;
  const Label *src_labels = pop_labels + static_cast<size_t>(parent) * n;
  const int *src_impr = pop_impr + static_cast<size_t>(parent) * n;
  int *impr = ls_impr + static_cast<size_t>(slot) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    s_labels[i] = src_labels[i];
    impr[i] = src_impr[i];
  }
  __syncthreads();

  const unsigned distance =
      LocalSearchBlock(adj, n, s_labels, impr, pop_dist[parent]);

  Label *labels = ls_labels + static_cast<size_t>(slot) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    labels[i] = s_labels[i];
  }
  if (threadIdx.x == 0) {
    ls_dist[slot] = distance;
  }
}

/**
 * Flip every vertex with probability p. Writes the new labels and their +-1
 * column for the GEMM; the result becomes the population of the next
 * iteration once FinishFromGemmKernel has run.
 */
__global__ void PerturbationKernel(const unsigned n, const unsigned n_pad,
                                   const float p,
                                   const Label *__restrict__ ls_labels,
                                   Label *__restrict__ pop_labels,
                                   int8_t *__restrict__ smat,
                                   Rng *__restrict__ rng) {
  const unsigned slot = blockIdx.x;
  const unsigned rng_id = slot * blockDim.x + threadIdx.x;
  Rng state = rng[rng_id];

  const Label *src = ls_labels + static_cast<size_t>(slot) * n;
  Label *labels = pop_labels + static_cast<size_t>(slot) * n;
  int8_t *s_column = smat + static_cast<size_t>(slot) * n_pad;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    Label label = src[i];
    if (curand_uniform(&state) <= p) {
      label = label == 0 ? 1 : 0;
    }
    labels[i] = label;
    s_column[i] = label == 0 ? 1 : -1;
  }
  rng[rng_id] = state;
}

/** Single local search from given labels (for ComputeLocalOptimum). */
__global__ void SingleLocalSearchKernel(const uint8_t *__restrict__ adj,
                                        const unsigned n,
                                        Label *__restrict__ labels,
                                        int *__restrict__ impr,
                                        unsigned *__restrict__ dist) {
  extern __shared__ Label s_labels[];
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    s_labels[i] = labels[i];
  }
  __syncthreads();
  unsigned distance = ComputeImprovements(adj, n, s_labels, impr);
  distance = LocalSearchBlock(adj, n, s_labels, impr, distance);
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    labels[i] = s_labels[i];
  }
  if (threadIdx.x == 0) {
    *dist = distance;
  }
}

}  // namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

non_strict_2cc::IPLSCudaAlgorithm::IPLSCudaAlgorithm(
    const unsigned iterations, const unsigned early_stop_num,
    IClustFactoryPtr factory, const unsigned population_size,
    const unsigned tournament_size, const double p_perturbation)
    : iterations_(iterations),
      early_stop_num_(early_stop_num),
      factory_(std::move(factory)),
      population_size_(population_size),
      tournament_size_(tournament_size),
      p_perturbation_(p_perturbation) {
  if (population_size_ == 0 || tournament_size_ == 0) {
    throw std::invalid_argument(
        "population_size and tournament_size must be positive");
  }
}

bool non_strict_2cc::IPLSCudaAlgorithm::IsAvailable() {
  return gcc_cuda::HasCudaDevice();
}

Solution non_strict_2cc::IPLSCudaAlgorithm::Train(
    const std::shared_ptr<IGraph> &graph) {
  const unsigned n = graph->Size();
  gcc_cuda::CheckGraphSize(n, kMaxVertices, kName);
  const unsigned n_pad = gcc_cuda::PadTo4(n);
  const unsigned threads = gcc_cuda::ThreadsForSize(n);
  const size_t shared_bytes = static_cast<size_t>(n) * sizeof(Label);
  const size_t pop_cells = static_cast<size_t>(population_size_) * n;
  const size_t pop_cells_pad = static_cast<size_t>(population_size_) * n_pad;

  DevArray<uint8_t> d_adj(static_cast<size_t>(n) * n);
  d_adj.Upload(gcc_cuda::FlattenGraph(*graph));
  // +-1 adjacency with zero diagonal, padded, for the GEMM.
  DevArray<int8_t> d_bmat(static_cast<size_t>(n_pad) * n_pad);
  gcc_cuda::BuildPaddedMatrix(d_adj.Get(), n, n_pad, 1, -1, 0, d_bmat.Get());
  // +-1 label columns (padding rows stay zero) and the GEMM output.
  DevArray<int8_t> d_smat(pop_cells_pad);
  GCC_CUDA_CHECK(cudaMemset(d_smat.Get(), 0, pop_cells_pad));
  DevArray<int> d_gmat(pop_cells_pad);
  gcc_cuda::CublasHandle cublas;

  DevArray<Label> d_pop_labels(pop_cells), d_ls_labels(pop_cells);
  DevArray<int> d_pop_impr(pop_cells), d_ls_impr(pop_cells);
  DevArray<unsigned> d_pop_dist(population_size_), d_ls_dist(population_size_);
  DevArray<Label> d_record(n);
  DevArray<Rng> d_rng(static_cast<size_t>(population_size_) * threads);

  auto finish_from_gemm = [&] {
    gcc_cuda::GemmInt8(cublas, n_pad, population_size_, n_pad, d_bmat.Get(),
                       d_smat.Get(), d_gmat.Get());
    FinishFromGemmKernel<<<population_size_, threads>>>(
        n, n_pad, d_pop_labels.Get(), d_gmat.Get(), d_pop_impr.Get(),
        d_pop_dist.Get());
    GCC_CUDA_CHECK(cudaGetLastError());
  };

  gcc_cuda::InitRngKernel<<<population_size_, threads>>>(
      d_rng.Get(), gcc_cuda::RandomSeed());
  GCC_CUDA_CHECK(cudaGetLastError());
  InitPopulationKernel<<<population_size_, threads>>>(
      n, n_pad, d_pop_labels.Get(), d_smat.Get(), d_rng.Get());
  GCC_CUDA_CHECK(cudaGetLastError());
  finish_from_gemm();

  std::vector<unsigned> h_ls_dist, h_pop_dist;
  unsigned record_dist = UINT_MAX;
  bool has_record = false;
  unsigned num_iter_without_record = 0;

  for (unsigned it = 0; it < iterations_; ++it) {
    SelectAndLocalSearchKernel<<<population_size_, threads, shared_bytes>>>(
        d_adj.Get(), n, population_size_, tournament_size_,
        d_pop_labels.Get(), d_pop_impr.Get(), d_pop_dist.Get(),
        d_ls_labels.Get(), d_ls_impr.Get(), d_ls_dist.Get(), d_rng.Get());
    GCC_CUDA_CHECK(cudaGetLastError());
    PerturbationKernel<<<population_size_, threads>>>(
        n, n_pad, static_cast<float>(p_perturbation_), d_ls_labels.Get(),
        d_pop_labels.Get(), d_smat.Get(), d_rng.Get());
    GCC_CUDA_CHECK(cudaGetLastError());
    finish_from_gemm();

    d_ls_dist.Download(h_ls_dist);
    d_pop_dist.Download(h_pop_dist);

    // Best of this iteration: local optima first, then perturbed solutions
    // (the CPU version keeps the first minimum in the same order).
    unsigned best_dist = UINT_MAX;
    const Label *best_labels = nullptr;
    for (unsigned s = 0; s < population_size_; ++s) {
      if (h_ls_dist[s] < best_dist) {
        best_dist = h_ls_dist[s];
        best_labels = d_ls_labels.Get() + static_cast<size_t>(s) * n;
      }
    }
    for (unsigned s = 0; s < population_size_; ++s) {
      if (h_pop_dist[s] < best_dist) {
        best_dist = h_pop_dist[s];
        best_labels = d_pop_labels.Get() + static_cast<size_t>(s) * n;
      }
    }

    if (has_record && best_dist >= record_dist) {
      num_iter_without_record++;
    } else {
      GCC_CUDA_CHECK(cudaMemcpy(d_record.Get(), best_labels,
                                static_cast<size_t>(n) * sizeof(Label),
                                cudaMemcpyDeviceToDevice));
      record_dist = best_dist;
      has_record = true;
      num_iter_without_record = 0;
    }
    if (num_iter_without_record == early_stop_num_) {
      break;
    }
  }

  std::vector<Label> h_record;
  d_record.Download(h_record);
  auto clustering = factory_->CreateClustering(n);
  for (unsigned i = 0; i < n; ++i) {
    clustering->SetupLabelForVertex(
        i, h_record[i] == 0 ? FIRST_CLUSTER : SECOND_CLUSTER);
  }
  return {record_dist, clustering};
}

IClustPtr non_strict_2cc::IPLSCudaAlgorithm::ComputeLocalOptimum(
    const IGraph &graph, const IClustPtr &cur_clustering) {
  const unsigned n = graph.Size();
  gcc_cuda::CheckGraphSize(n, kMaxVertices, kName);
  if (cur_clustering->Size() != n) {
    throw std::invalid_argument("clustering size must match graph size");
  }
  const unsigned threads = gcc_cuda::ThreadsForSize(n);

  std::vector<Label> h_labels(n);
  for (unsigned i = 0; i < n; ++i) {
    const auto label = cur_clustering->GetLabel(i);
    if (label != FIRST_CLUSTER && label != SECOND_CLUSTER) {
      throw std::invalid_argument("every vertex must be labeled 0 or 1");
    }
    h_labels[i] = label == FIRST_CLUSTER ? 0 : 1;
  }

  DevArray<uint8_t> d_adj(static_cast<size_t>(n) * n);
  d_adj.Upload(gcc_cuda::FlattenGraph(graph));
  DevArray<Label> d_labels(n);
  d_labels.Upload(h_labels);
  DevArray<int> d_impr(n);
  DevArray<unsigned> d_dist(1);

  SingleLocalSearchKernel<<<1, threads, static_cast<size_t>(n)>>>(
      d_adj.Get(), n, d_labels.Get(), d_impr.Get(), d_dist.Get());
  GCC_CUDA_CHECK(cudaGetLastError());
  d_labels.Download(h_labels);

  auto result = cur_clustering->GetCopy();
  for (unsigned i = 0; i < n; ++i) {
    result->SetupLabelForVertex(
        i, h_labels[i] == 0 ? FIRST_CLUSTER : SECOND_CLUSTER);
  }
  return result;
}
