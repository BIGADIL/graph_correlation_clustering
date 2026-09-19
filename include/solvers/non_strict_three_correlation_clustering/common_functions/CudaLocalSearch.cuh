#pragma once

// Device-side building blocks of the NS3CC algorithms: move gains, the greedy
// 3-label local search, distance and the neighborhood splits. Included only
// from .cu files; all functions are executed by a whole block that owns one
// clustering whose labels live in shared memory.
//
// gains[i * 3 + m] is the decrease of the distance when vertex i is moved to
// label m (LocalSearch::ComputeLocalImprovement); gains[i * 3 + label(i)] is
// kept at 0 and never read. With A_k(i) = sum over j != i with label k of
// (joined(i, j) ? +1 : -1) the gain is A_m(i) - A_label(i)(i).

#include <climits>
#include <cstdint>

#include "../../../common/CudaCommon.cuh"

namespace non_strict_3cc::cuda {

using gcc_cuda::Label;

constexpr unsigned kNumLabels = 3;

/**
 * Compute the gains of all vertices for the labels in shared memory and the
 * distance of the clustering to the graph. O(n^2) work split by vertex;
 * thread i reads column i, so for a fixed j the block reads row j contiguously.
 */
__device__ inline unsigned ComputeGains(const uint8_t *__restrict__ adj,
                                        const unsigned n,
                                        const Label *__restrict__ s_labels,
                                        int *__restrict__ gains) {
  long long disagreements = 0;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    const Label li = s_labels[i];
    int a[kNumLabels] = {0, 0, 0};
    int dis = 0;
    for (unsigned j = 0; j < n; ++j) {
      if (j == i) {
        continue;
      }
      const bool joined = adj[static_cast<size_t>(j) * n + i] != 0;
      const Label lj = s_labels[j];
      a[lj] += joined ? 1 : -1;
      dis += ((lj == li) != joined) ? 1 : 0;
    }
    for (unsigned m = 0; m < kNumLabels; ++m) {
      gains[i * kNumLabels + m] =
          m == static_cast<unsigned>(li) ? 0 : a[m] - a[li];
    }
    disagreements += dis;
  }
  disagreements = gcc_cuda::BlockSum(disagreements);
  return static_cast<unsigned>(disagreements / 2);
}

/** Distance of the labels in shared memory to the graph, O(n^2) per block. */
__device__ inline unsigned ComputeDistance(const uint8_t *__restrict__ adj,
                                           const unsigned n,
                                           const Label *__restrict__ s_labels) {
  long long disagreements = 0;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    const Label li = s_labels[i];
    int dis = 0;
    for (unsigned j = 0; j < n; ++j) {
      const bool joined = adj[static_cast<size_t>(j) * n + i] != 0;
      dis += ((s_labels[j] == li) != joined) ? 1 : 0;
    }
    // The zero diagonal counts as "same cluster, no edge": remove it.
    disagreements += dis - 1;
  }
  disagreements = gcc_cuda::BlockSum(disagreements);
  return static_cast<unsigned>(disagreements / 2);
}

/**
 * Move vertex v to new_label (gain = gains[v][new_label]) and update the
 * gains of every vertex (LocalSearch::UpdateLocalImprovements). The labels
 * used for the update are those *before* the move, as on the CPU.
 */
__device__ inline void MoveVertex(const uint8_t *__restrict__ adj,
                                  const unsigned n,
                                  Label *__restrict__ s_labels,
                                  int *__restrict__ gains, const unsigned v,
                                  const Label new_label, const int gain) {
  const Label old_label = s_labels[v];
  const auto third_label =
      static_cast<Label>(kNumLabels - old_label - new_label);
  const uint8_t *row = adj + static_cast<size_t>(v) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    if (i == v) {
      continue;
    }
    const int sign = row[i] != 0 ? 1 : -1;
    const Label li = s_labels[i];
    int *g = gains + i * kNumLabels;
    if (li == old_label) {
      g[new_label] += 2 * sign;
      g[third_label] += sign;
    } else if (li == new_label) {
      g[old_label] -= 2 * sign;
      g[third_label] -= sign;
    } else {
      g[old_label] -= sign;
      g[new_label] += sign;
    }
  }
  __syncthreads();
  if (threadIdx.x == 0) {
    // A_k(v) does not depend on the label of v, so the gains of v after the
    // move follow from the gains before it.
    int *g = gains + v * kNumLabels;
    g[old_label] = -gain;
    g[third_label] -= gain;
    g[new_label] = 0;
    s_labels[v] = new_label;
  }
  __syncthreads();
}

/**
 * Greedy 3-label local search (LocalSearch::ComputeLocalOptimum) for the
 * labels in shared memory with precomputed gains. Returns the new distance.
 */
__device__ inline unsigned LocalSearchBlock(const uint8_t *__restrict__ adj,
                                            const unsigned n,
                                            Label *__restrict__ s_labels,
                                            int *__restrict__ gains,
                                            unsigned distance) {
  while (true) {
    int best = INT_MIN;
    unsigned candidate = UINT_MAX;
    for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
      const Label li = s_labels[i];
      for (unsigned m = 0; m < kNumLabels; ++m) {
        if (m == static_cast<unsigned>(li)) {
          continue;
        }
        const int g = gains[i * kNumLabels + m];
        if (g > best) {
          best = g;
          candidate = i * kNumLabels + m;
        }
      }
    }
    int g_best;
    unsigned idx;
    gcc_cuda::BlockArgMax(best, candidate, g_best, idx);
    if (g_best <= 0) {
      break;
    }
    MoveVertex(adj, n, s_labels, gains, idx / kNumLabels,
               static_cast<Label>(idx % kNumLabels), g_best);
    distance -= static_cast<unsigned>(g_best);
  }
  return distance;
}

/**
 * Split by two vertices (NeighborSplitter::SplitGraphByTwoVertices): first
 * and its neighbors go to the first cluster, second and its remaining
 * neighbors to the second, everything else to the third.
 */
__device__ inline void SplitByTwoVertices(const uint8_t *__restrict__ adj,
                                          const unsigned n,
                                          const unsigned first,
                                          const unsigned second,
                                          Label *__restrict__ s_labels) {
  const uint8_t *row1 = adj + static_cast<size_t>(first) * n;
  const uint8_t *row2 = adj + static_cast<size_t>(second) * n;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    // `second` stays in the second cluster even if it is joined to `first`.
    Label label;
    if (i == first) {
      label = 0;
    } else if (i == second) {
      label = 1;
    } else if (row1[i] != 0) {
      label = 0;
    } else if (row2[i] != 0) {
      label = 1;
    } else {
      label = 2;
    }
    s_labels[i] = label;
  }
  __syncthreads();
}

}  // namespace non_strict_3cc::cuda
