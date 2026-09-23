#pragma once

// Device-side building blocks of the NS2CC algorithms: local improvements,
// the greedy local search of Coleman, Saunderson and Wirth, and the
// neighborhood split. Included only from .cu files; all functions are
// executed by a whole block that owns one clustering.

#include <climits>
#include <cstdint>

#include "../../../common/CudaCommon.cuh"

namespace non_strict_2cc::cuda {

using gcc_cuda::Label;

/**
 * Contribution of the pair (i, j) to the local improvement of both vertices:
 * +1 if the pair is a disagreement (flipping either vertex fixes it),
 * -1 if the pair agrees (flipping either vertex breaks it).
 */
__device__ __forceinline__ int PairTerm(const uint8_t joined, const Label li, const Label lj) {
  return ((li == lj) != (joined != 0)) ? 1 : -1;
}

/**
 * Compute local improvements of all vertices for the labels in shared memory
 * (equivalent of LocalSearch::InitLocalImprovements) and the distance of the
 * clustering to the graph. O(n^2) work, split between the threads by vertex.
 *
 * Thread i reads adj[j][i] for all j; for a fixed j the threads of the block
 * read a contiguous part of row j, so the access is coalesced.
 */
__device__ inline unsigned ComputeImprovements(const uint8_t* __restrict__ adj, const unsigned n,
                                               const Label* __restrict__ s_labels, int* __restrict__ impr) {
  long long total = 0;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    const Label li = s_labels[i];
    int g = 0;
    for (unsigned j = 0; j < n; ++j) {
      g += PairTerm(adj[static_cast<size_t>(j) * n + i], li, s_labels[j]);
    }
    // The diagonal is zero and contributes +1 for "same cluster, no edge":
    // remove it.
    g -= 1;
    impr[i] = g;
    total += g;
  }
  total = gcc_cuda::BlockSum(total);
  // sum_i impr[i] = 2 * (disagreements - agreements) = 4 * D - n * (n - 1).
  const long long pairs2 = static_cast<long long>(n) * (n - 1);
  return static_cast<unsigned>((total + pairs2) / 4);
}

/**
 * Local improvements and distance from a GEMM column.
 *
 * With s_i = +1 for the first cluster and -1 for the second and B the +-1
 * adjacency matrix with zero diagonal, the improvement of vertex i is
 * -s_i * (B s)_i. `g` is the column (B s) for this clustering (int32, at
 * least n entries). Writes impr[i] and returns the distance; `impr` may alias
 * `g`.
 */
__device__ inline unsigned ImprovementsFromGemm(const unsigned n, const Label* __restrict__ s_labels, const int* g,
                                                int* impr) {
  long long total = 0;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    const int gi = g[i];
    const int value = s_labels[i] == 0 ? -gi : gi;
    impr[i] = value;
    total += value;
  }
  total = gcc_cuda::BlockSum(total);
  const long long pairs2 = static_cast<long long>(n) * (n - 1);
  return static_cast<unsigned>((total + pairs2) / 4);
}

/** Write the +-1 label column used by the GEMM (padding stays untouched). */
__device__ inline void WriteSignedLabels(const unsigned n, const Label* __restrict__ s_labels,
                                         int8_t* __restrict__ s_column) {
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    s_column[i] = s_labels[i] == 0 ? 1 : -1;
  }
}

/**
 * Greedy local search (LocalSearch::ComputeLocalOptimum) for the labels in
 * shared memory with precomputed improvements. Returns the new distance.
 *
 * Every step makes one pass over the improvements: while the flip of v is
 * applied to impr[i], each thread also tracks the maximum of the new values,
 * so the candidate of the next step comes for free. The flipped vertex itself
 * gets impr[v] = -gain < 0 and can never be the strictly positive maximum
 * the loop continues on, so leaving it out of the scan does not change the
 * result. Ties go to the smallest vertex, as in the CPU scan.
 */
__device__ inline unsigned LocalSearchBlock(const uint8_t* __restrict__ adj, const unsigned n,
                                            Label* __restrict__ s_labels, int* __restrict__ impr, unsigned distance) {
  int best = INT_MIN;
  unsigned candidate = UINT_MAX;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    const int g = impr[i];
    if (g > best) {
      best = g;
      candidate = i;
    }
  }
  while (true) {
    int g_best;
    unsigned v;
    gcc_cuda::BlockArgMax(best, candidate, g_best, v);
    if (g_best <= 0) {
      break;
    }

    // Update improvements with labels *before* the flip, as the CPU does,
    // and scan the new values for the next candidate in the same pass.
    const Label lv = s_labels[v];
    const uint8_t* row = adj + static_cast<size_t>(v) * n;
    best = INT_MIN;
    candidate = UINT_MAX;
    for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
      if (i == v) {
        continue;
      }
      const int g = impr[i] - 2 * PairTerm(row[i], s_labels[i], lv);
      impr[i] = g;
      if (g > best) {
        best = g;
        candidate = i;
      }
    }
    __syncthreads();
    if (threadIdx.x == 0) {
      impr[v] = -g_best;
      s_labels[v] = lv == 0 ? 1 : 0;
    }
    distance -= static_cast<unsigned>(g_best);
    // BlockArgMax at the top of the loop synchronizes before anyone reads
    // s_labels[v] or impr[v] again.
  }
  return distance;
}

/**
 * Neighborhood split of the graph by vertex v
 * (NeighborSplitter::SplitGraphByVertex): v and its neighbors go to the first
 * cluster, everything else to the second.
 */
__device__ inline void SplitByVertex(const uint8_t* __restrict__ adj, const unsigned n, const unsigned v,
                                     Label* __restrict__ s_labels) {
  const uint8_t* row = adj + static_cast<size_t>(v) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    s_labels[i] = (i == v || row[i] != 0) ? 0 : 1;
  }
  __syncthreads();
}

}  // namespace non_strict_2cc::cuda
