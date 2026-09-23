#include <climits>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../../../include/common/CudaCommon.cuh"
#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/CudaLocalSearch.cuh"
#include "../../../../include/solvers/non_strict_three_correlation_clustering/ipls_algorithms/IPLSCudaAlgorithm.hpp"

namespace {

using gcc_cuda::DevArray;
using gcc_cuda::Label;
using gcc_cuda::Rng;
using non_strict_3cc::cuda::ComputeGains;
using non_strict_3cc::cuda::kNumLabels;
using non_strict_3cc::cuda::LocalSearchBlock;
using non_strict_3cc::cuda::MoveVertexFinalize;
using non_strict_3cc::cuda::MoveVertexUpdateOthers;
// Dynamic shared memory holds one label and one perturbation flag per vertex.
constexpr unsigned kMaxVertices = gcc_cuda::kSharedBudget / 2;
constexpr char kName[] = "non_strict_3cc::IPLSCudaAlgorithm";

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

// ---------------------------------------------------------------------------
// Device helpers
// ---------------------------------------------------------------------------

/**
 * Perturbation (IPLSAlgorithm::Perturbation): scan the vertices in order and
 * move each flagged vertex to the best of the two other labels given the
 * current labels. Gains and distance stay consistent. Returns the distance.
 */
__device__ unsigned PerturbBlock(const uint8_t* __restrict__ adj, const unsigned n, Label* __restrict__ s_labels,
                                 const Label* __restrict__ s_flags, int* __restrict__ gains, unsigned distance) {
  for (unsigned v = 0; v < n; ++v) {
    if (!s_flags[v]) {
      continue;
    }
    // Every thread evaluates the same three cells, so the decision is
    // uniform across the block without an extra broadcast.
    int best;
    Label new_label;
    non_strict_3cc::cuda::BestMoveOf(gains + v * kNumLabels, s_labels[v], best, new_label);
    int unused_best;
    unsigned unused_cand;
    MoveVertexUpdateOthers(adj, n, s_labels, gains, v, new_label, unused_best, unused_cand);
    __syncthreads();
    MoveVertexFinalize(s_labels, gains, v, new_label, best);
    __syncthreads();
    distance = static_cast<unsigned>(static_cast<int>(distance) - best);
  }
  return distance;
}

// ---------------------------------------------------------------------------
// Kernels. One block per individual; blockIdx.x is the slot in the population.
// ---------------------------------------------------------------------------

__global__ void InitPopulationKernel(const uint8_t* __restrict__ adj, const unsigned n, Label* __restrict__ pop_labels,
                                     int* __restrict__ pop_gains, unsigned* __restrict__ pop_dist,
                                     Rng* __restrict__ rng) {
  extern __shared__ Label s_labels[];
  const unsigned slot = blockIdx.x;
  const unsigned rng_id = slot * blockDim.x + threadIdx.x;
  Rng state = rng[rng_id];

  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    s_labels[i] = static_cast<Label>(curand(&state) % kNumLabels);
  }
  rng[rng_id] = state;
  __syncthreads();

  Label* labels = pop_labels + static_cast<size_t>(slot) * n;
  int* gains = pop_gains + static_cast<size_t>(slot) * n * kNumLabels;
  const unsigned distance = ComputeGains(adj, n, s_labels, gains);
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    labels[i] = s_labels[i];
  }
  if (threadIdx.x == 0) {
    pop_dist[slot] = distance;
  }
}

/**
 * Tournament selection from the population, then local search.
 * Writes the local optimum into ls_* at the same slot.
 */
__global__ void SelectAndLocalSearchKernel(const uint8_t* __restrict__ adj, const unsigned n,
                                           const unsigned population_size, const unsigned tournament_size,
                                           const Label* __restrict__ pop_labels, const int* __restrict__ pop_gains,
                                           const unsigned* __restrict__ pop_dist, Label* __restrict__ ls_labels,
                                           int* __restrict__ ls_gains, unsigned* __restrict__ ls_dist,
                                           Rng* __restrict__ rng) {
  extern __shared__ Label s_labels[];
  __shared__ unsigned s_parent;
  const unsigned slot = blockIdx.x;

  if (threadIdx.x == 0) {
    const unsigned rng_id = slot * blockDim.x;
    Rng state = rng[rng_id];
    s_parent = gcc_cuda::Tournament(pop_dist, population_size, tournament_size, state);
    rng[rng_id] = state;
  }
  __syncthreads();

  const unsigned parent = s_parent;
  const Label* src_labels = pop_labels + static_cast<size_t>(parent) * n;
  const int* src_gains = pop_gains + static_cast<size_t>(parent) * n * kNumLabels;
  int* gains = ls_gains + static_cast<size_t>(slot) * n * kNumLabels;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    s_labels[i] = src_labels[i];
  }
  for (unsigned i = threadIdx.x; i < n * kNumLabels; i += blockDim.x) {
    gains[i] = src_gains[i];
  }
  __syncthreads();

  const unsigned distance = LocalSearchBlock(adj, n, s_labels, gains, pop_dist[parent]);

  Label* labels = ls_labels + static_cast<size_t>(slot) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    labels[i] = s_labels[i];
  }
  if (threadIdx.x == 0) {
    ls_dist[slot] = distance;
  }
}

/**
 * Perturb the local optimum of the slot; the result becomes the population
 * of the next iteration. Shared memory: n labels followed by n flags.
 */
__global__ void PerturbationKernel(const uint8_t* __restrict__ adj, const unsigned n, const float p,
                                   const Label* __restrict__ ls_labels, const int* __restrict__ ls_gains,
                                   Label* __restrict__ pop_labels, int* __restrict__ pop_gains,
                                   unsigned* __restrict__ pop_dist, const unsigned* __restrict__ ls_dist,
                                   Rng* __restrict__ rng) {
  extern __shared__ Label s_mem[];
  Label* s_labels = s_mem;
  Label* s_flags = s_mem + n;
  const unsigned slot = blockIdx.x;
  const unsigned rng_id = slot * blockDim.x + threadIdx.x;
  Rng state = rng[rng_id];

  const Label* src_labels = ls_labels + static_cast<size_t>(slot) * n;
  const int* src_gains = ls_gains + static_cast<size_t>(slot) * n * kNumLabels;
  int* gains = pop_gains + static_cast<size_t>(slot) * n * kNumLabels;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    s_labels[i] = src_labels[i];
    s_flags[i] = curand_uniform(&state) <= p ? 1 : 0;
  }
  for (unsigned i = threadIdx.x; i < n * kNumLabels; i += blockDim.x) {
    gains[i] = src_gains[i];
  }
  rng[rng_id] = state;
  __syncthreads();

  const unsigned distance = PerturbBlock(adj, n, s_labels, s_flags, gains, ls_dist[slot]);

  Label* labels = pop_labels + static_cast<size_t>(slot) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    labels[i] = s_labels[i];
  }
  if (threadIdx.x == 0) {
    pop_dist[slot] = distance;
  }
}

/** Single local search from given labels (for ComputeLocalOptimum). */
__global__ void SingleLocalSearchKernel(const uint8_t* __restrict__ adj, const unsigned n, Label* __restrict__ labels,
                                        int* __restrict__ gains, unsigned* __restrict__ dist) {
  extern __shared__ Label s_labels[];
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    s_labels[i] = labels[i];
  }
  __syncthreads();
  unsigned distance = ComputeGains(adj, n, s_labels, gains);
  distance = LocalSearchBlock(adj, n, s_labels, gains, distance);
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

non_strict_3cc::IPLSCudaAlgorithm::IPLSCudaAlgorithm(const unsigned iterations, const unsigned early_stop_num,
                                                     IClustFactoryPtr factory, const unsigned population_size,
                                                     const unsigned tournament_size, const double p_perturbation)
    : iterations_(iterations),
      early_stop_num_(early_stop_num),
      factory_(std::move(factory)),
      population_size_(population_size),
      tournament_size_(tournament_size),
      p_perturbation_(p_perturbation) {
  if (population_size_ == 0 || tournament_size_ == 0) {
    throw std::invalid_argument("population_size and tournament_size must be positive");
  }
}

bool non_strict_3cc::IPLSCudaAlgorithm::IsAvailable() { return gcc_cuda::HasCudaDevice(); }

Solution non_strict_3cc::IPLSCudaAlgorithm::Train(const std::shared_ptr<IGraph>& graph) {
  const unsigned n = graph->Size();
  gcc_cuda::CheckGraphSize(n, kMaxVertices, kName);
  const unsigned threads = gcc_cuda::ThreadsForSize(n);
  const size_t labels_bytes = static_cast<size_t>(n) * sizeof(Label);
  const size_t pop_cells = static_cast<size_t>(population_size_) * n;

  DevArray<uint8_t> d_adj(static_cast<size_t>(n) * n);
  d_adj.Upload(gcc_cuda::FlattenGraph(*graph));

  DevArray<Label> d_pop_labels(pop_cells), d_ls_labels(pop_cells);
  DevArray<int> d_pop_gains(pop_cells * kNumLabels), d_ls_gains(pop_cells * kNumLabels);
  DevArray<unsigned> d_pop_dist(population_size_), d_ls_dist(population_size_);
  DevArray<Label> d_record(n);
  DevArray<Rng> d_rng(static_cast<size_t>(population_size_) * threads);

  gcc_cuda::InitRngKernel<<<population_size_, threads>>>(d_rng.Get(), gcc_cuda::RandomSeed());
  GCC_CUDA_CHECK(cudaGetLastError());
  InitPopulationKernel<<<population_size_, threads, labels_bytes>>>(d_adj.Get(), n, d_pop_labels.Get(),
                                                                    d_pop_gains.Get(), d_pop_dist.Get(), d_rng.Get());
  GCC_CUDA_CHECK(cudaGetLastError());

  std::vector<unsigned> h_ls_dist, h_pop_dist;
  unsigned record_dist = UINT_MAX;
  bool has_record = false;
  unsigned num_iter_without_record = 0;

  for (unsigned it = 0; it < iterations_; ++it) {
    SelectAndLocalSearchKernel<<<population_size_, threads, labels_bytes>>>(
        d_adj.Get(), n, population_size_, tournament_size_, d_pop_labels.Get(), d_pop_gains.Get(), d_pop_dist.Get(),
        d_ls_labels.Get(), d_ls_gains.Get(), d_ls_dist.Get(), d_rng.Get());
    GCC_CUDA_CHECK(cudaGetLastError());
    PerturbationKernel<<<population_size_, threads, 2 * labels_bytes>>>(
        d_adj.Get(), n, static_cast<float>(p_perturbation_), d_ls_labels.Get(), d_ls_gains.Get(), d_pop_labels.Get(),
        d_pop_gains.Get(), d_pop_dist.Get(), d_ls_dist.Get(), d_rng.Get());
    GCC_CUDA_CHECK(cudaGetLastError());

    d_ls_dist.Download(h_ls_dist);
    d_pop_dist.Download(h_pop_dist);

    // Best of this iteration: local optima first, then perturbed solutions
    // (the CPU version keeps the first minimum in the same order).
    unsigned best_dist = UINT_MAX;
    const Label* best_labels = nullptr;
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
      GCC_CUDA_CHECK(cudaMemcpy(d_record.Get(), best_labels, labels_bytes, cudaMemcpyDeviceToDevice));
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
    clustering->SetupLabelForVertex(i, ToClusterLabel(h_record[i]));
  }
  return {record_dist, clustering};
}

IClustPtr non_strict_3cc::IPLSCudaAlgorithm::ComputeLocalOptimum(const IGraph& graph, const IClustPtr& cur_clustering) {
  const unsigned n = graph.Size();
  gcc_cuda::CheckGraphSize(n, kMaxVertices, kName);
  if (cur_clustering->Size() != n) {
    throw std::invalid_argument("clustering size must match graph size");
  }
  const unsigned threads = gcc_cuda::ThreadsForSize(n);

  std::vector<Label> h_labels(n);
  for (unsigned i = 0; i < n; ++i) {
    const auto label = cur_clustering->GetLabel(i);
    if (label != FIRST_CLUSTER && label != SECOND_CLUSTER && label != THIRD_CLUSTER) {
      throw std::invalid_argument("every vertex must be labeled 0, 1 or 2");
    }
    h_labels[i] = static_cast<Label>(label);
  }

  DevArray<uint8_t> d_adj(static_cast<size_t>(n) * n);
  d_adj.Upload(gcc_cuda::FlattenGraph(graph));
  DevArray<Label> d_labels(n);
  d_labels.Upload(h_labels);
  DevArray<int> d_gains(static_cast<size_t>(n) * kNumLabels);
  DevArray<unsigned> d_dist(1);

  SingleLocalSearchKernel<<<1, threads, static_cast<size_t>(n)>>>(d_adj.Get(), n, d_labels.Get(), d_gains.Get(),
                                                                  d_dist.Get());
  GCC_CUDA_CHECK(cudaGetLastError());
  d_labels.Download(h_labels);

  auto result = cur_clustering->GetCopy();
  for (unsigned i = 0; i < n; ++i) {
    result->SetupLabelForVertex(i, ToClusterLabel(h_labels[i]));
  }
  return result;
}
