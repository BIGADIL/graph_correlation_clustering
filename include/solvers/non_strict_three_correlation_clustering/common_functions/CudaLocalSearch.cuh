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
__device__ inline unsigned ComputeGains(const uint8_t* __restrict__ adj, const unsigned n,
                                        const Label* __restrict__ s_labels, int* __restrict__ gains) {
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
      gains[i * kNumLabels + m] = m == static_cast<unsigned>(li) ? 0 : a[m] - a[li];
    }
    disagreements += dis;
  }
  disagreements = gcc_cuda::BlockSum(disagreements);
  return static_cast<unsigned>(disagreements / 2);
}

/** Distance of the labels in shared memory to the graph, O(n^2) per block. */
__device__ inline unsigned ComputeDistance(const uint8_t* __restrict__ adj, const unsigned n,
                                           const Label* __restrict__ s_labels) {
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
 * Gains and distance from GEMM columns.
 *
 * `g` points at three consecutive int32 columns of stride `ld`: column k is
 * (A L_k), the number of neighbors of every vertex inside cluster k, where A
 * is the 0/1 adjacency matrix and L_k the indicator of cluster k. With
 * A_k(i) = sum over j != i in cluster k of (joined ? +1 : -1)
 *        = 2 * (A L_k)_i - (n_k - [label(i) == k])
 * the gain of moving i to m is A_m(i) - A_label(i)(i). The distance is
 * sum_k C(n_k, 2) + m_edges - 2 * intra, intra = 1/2 * sum_i (A L_label(i))_i.
 * Label counts are reduced over the block here. `gains` may be nullptr when
 * only the distance is needed.
 */
__device__ inline unsigned GainsFromGemm(const unsigned n, const Label* __restrict__ s_labels,
                                         const int* __restrict__ g, const unsigned ld,
                                         const unsigned long long num_edges, int* __restrict__ gains) {
  long long count[kNumLabels] = {0, 0, 0};
  long long twice_intra = 0;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    const Label li = s_labels[i];
    count[li] += 1;
    twice_intra += g[static_cast<size_t>(li) * ld + i];
  }
  long long total[kNumLabels];
  for (unsigned k = 0; k < kNumLabels; ++k) {
    total[k] = gcc_cuda::BlockSum(count[k]);
  }
  twice_intra = gcc_cuda::BlockSum(twice_intra);
  if (gains != nullptr) {
    for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
      const Label li = s_labels[i];
      int a[kNumLabels];
      for (unsigned k = 0; k < kNumLabels; ++k) {
        const int others = static_cast<int>(total[k]) - (li == static_cast<Label>(k) ? 1 : 0);
        a[k] = 2 * g[static_cast<size_t>(k) * ld + i] - others;
      }
      for (unsigned m = 0; m < kNumLabels; ++m) {
        gains[i * kNumLabels + m] = m == static_cast<unsigned>(li) ? 0 : a[m] - a[li];
      }
    }
  }
  long long distance = static_cast<long long>(num_edges) - twice_intra;
  for (unsigned k = 0; k < kNumLabels; ++k) {
    distance += total[k] * (total[k] - 1) / 2;
  }
  return static_cast<unsigned>(distance);
}

/**
 * Best move of vertex i given its three gains: the largest gain over the two
 * labels other than li; ties go to the smaller label, as in the CPU scan.
 */
__device__ __forceinline__ void BestMoveOf(const int* __restrict__ g, const Label li, int& best, Label& label) {
  best = INT_MIN;
  label = 0;
  for (unsigned m = 0; m < kNumLabels; ++m) {
    if (m == static_cast<unsigned>(li)) {
      continue;
    }
    if (g[m] > best) {
      best = g[m];
      label = static_cast<Label>(m);
    }
  }
}

/**
 * First half of a move of vertex v to new_label: update the gains of every
 * other vertex (LocalSearch::UpdateLocalImprovements) using the labels
 * *before* the move, as on the CPU. In the same pass each thread tracks the
 * best move among its vertices (candidate = i * 3 + m) for the next local
 * search step. Must be followed by a block barrier and MoveVertexFinalize.
 */
__device__ inline void MoveVertexUpdateOthers(const uint8_t* __restrict__ adj, const unsigned n,
                                              const Label* __restrict__ s_labels, int* __restrict__ gains,
                                              const unsigned v, const Label new_label, int& best, unsigned& candidate) {
  const Label old_label = s_labels[v];
  const Label third_label = static_cast<Label>(kNumLabels - old_label - new_label);
  const uint8_t* row = adj + static_cast<size_t>(v) * n;
  best = INT_MIN;
  candidate = UINT_MAX;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    if (i == v) {
      continue;
    }
    const int sign = row[i] != 0 ? 1 : -1;
    const Label li = s_labels[i];
    int* g = gains + i * kNumLabels;
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
    int local_best;
    Label local_label;
    BestMoveOf(g, li, local_best, local_label);
    if (local_best > best) {
      best = local_best;
      candidate = i * kNumLabels + local_label;
    }
  }
}

/**
 * Second half of a move: thread 0 sets the gains and the label of v itself.
 * A_k(v) does not depend on the label of v, so the gains of v after the move
 * follow from the gains before it. The caller must place a block barrier
 * before the labels or gains of v are read again.
 */
__device__ inline void MoveVertexFinalize(Label* __restrict__ s_labels, int* __restrict__ gains, const unsigned v,
                                          const Label new_label, const int gain) {
  if (threadIdx.x == 0) {
    const Label old_label = s_labels[v];
    const Label third_label = static_cast<Label>(kNumLabels - old_label - new_label);
    int* g = gains + v * kNumLabels;
    g[old_label] = -gain;
    g[third_label] -= gain;
    g[new_label] = 0;
    s_labels[v] = new_label;
  }
}

/**
 * Greedy 3-label local search (LocalSearch::ComputeLocalOptimum) for the
 * labels in shared memory with precomputed gains. Returns the new distance.
 *
 * Every step makes one pass over the gains: the update of the move and the
 * scan for the next candidate are fused. The moved vertex is left out of the
 * scan: after a move to its best label its own gains are all <= 0, so it can
 * never be the strictly positive maximum the loop continues on. The CPU scans
 * vertices, then labels, in ascending order and keeps the first maximum,
 * i.e. the smallest (i * 3 + m) among the maxima; BlockArgMax does the same.
 */
__device__ inline unsigned LocalSearchBlock(const uint8_t* __restrict__ adj, const unsigned n,
                                            Label* __restrict__ s_labels, int* __restrict__ gains, unsigned distance) {
  int best = INT_MIN;
  unsigned candidate = UINT_MAX;
  for (unsigned i = threadIdx.x; i < n; i += blockDim.x) {
    int local_best;
    Label local_label;
    BestMoveOf(gains + i * kNumLabels, s_labels[i], local_best, local_label);
    if (local_best > best) {
      best = local_best;
      candidate = i * kNumLabels + local_label;
    }
  }
  while (true) {
    int g_best;
    unsigned idx;
    gcc_cuda::BlockArgMax(best, candidate, g_best, idx);
    if (g_best <= 0) {
      break;
    }
    const unsigned v = idx / kNumLabels;
    const Label new_label = static_cast<Label>(idx % kNumLabels);
    MoveVertexUpdateOthers(adj, n, s_labels, gains, v, new_label, best, candidate);
    __syncthreads();
    MoveVertexFinalize(s_labels, gains, v, new_label, g_best);
    distance -= static_cast<unsigned>(g_best);
    // BlockArgMax at the top of the loop synchronizes before anyone reads
    // the labels or gains of v again.
  }
  return distance;
}

/**
 * Split by two vertices (NeighborSplitter::SplitGraphByTwoVertices): first
 * and its neighbors go to the first cluster, second and its remaining
 * neighbors to the second, everything else to the third.
 */
__device__ inline void SplitByTwoVertices(const uint8_t* __restrict__ adj, const unsigned n, const unsigned first,
                                          const unsigned second, Label* __restrict__ s_labels) {
  const uint8_t* row1 = adj + static_cast<size_t>(first) * n;
  const uint8_t* row2 = adj + static_cast<size_t>(second) * n;
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
