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
__device__ __forceinline__ int PairTerm(const uint8_t joined, const Label li,
                                        const Label lj) {
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
__device__ inline unsigned ComputeImprovements(
    const uint8_t *__restrict__ adj, const unsigned n,
    const Label *__restrict__ s_labels, int *__restrict__ impr) {
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
 * Greedy local search (LocalSearch::ComputeLocalOptimum) for the labels in
 * shared memory with precomputed improvements. Returns the new distance.
 */
__device__ inline unsigned LocalSearchBlock(const uint8_t *__restrict__ adj,
                                            const unsigned n,
                                            Label *__restrict__ s_labels,
                                            int *__restrict__ impr,
                                            unsigned distance) {
  while (true) {
    int best = INT_MIN;
    unsigned candidate = UINT_MAX;
    for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
      const int g = impr[i];
      if (g > best) {
        best = g;
        candidate = i;
      }
    }
    int g_best;
    unsigned v;
    gcc_cuda::BlockArgMax(best, candidate, g_best, v);
    if (g_best <= 0) {
      break;
    }

    // Update improvements with labels *before* the flip, as the CPU does.
    const Label lv = s_labels[v];
    const uint8_t *row = adj + static_cast<size_t>(v) * n;
    for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
      if (i != v) {
        impr[i] -= 2 * PairTerm(row[i], s_labels[i], lv);
      }
    }
    __syncthreads();
    if (threadIdx.x == 0) {
      impr[v] = -g_best;
      s_labels[v] = lv == 0 ? 1 : 0;
    }
    distance -= static_cast<unsigned>(g_best);
    __syncthreads();
  }
  return distance;
}

/**
 * Neighborhood split of the graph by vertex v
 * (NeighborSplitter::SplitGraphByVertex): v and its neighbors go to the first
 * cluster, everything else to the second.
 */
__device__ inline void SplitByVertex(const uint8_t *__restrict__ adj,
                                     const unsigned n, const unsigned v,
                                     Label *__restrict__ s_labels) {
  const uint8_t *row = adj + static_cast<size_t>(v) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    s_labels[i] = (i == v || row[i] != 0) ? 0 : 1;
  }
  __syncthreads();
}

}  // namespace non_strict_2cc::cuda
