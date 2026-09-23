#pragma once

#include <cublas_v2.h>
#include <cuda_runtime.h>
#include <curand_kernel.h>

#include <algorithm>
#include <climits>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "../graphs/IGraph.hpp"

namespace gcc_cuda {
#define GCC_CUDA_CHECK(call)                                                                    \
  do {                                                                                          \
    const cudaError_t err__ = (call);                                                           \
    if (err__ != cudaSuccess) {                                                                 \
      throw std::runtime_error(std::string("CUDA error: ") + cudaGetErrorString(err__) + " (" + \
                               std::string(__FILE__) + ":" + std::to_string(__LINE__) + ")");   \
    }                                                                                           \
  } while (0)

/** Owning device array. */
template <typename T>
class DevArray {
  T* ptr_ = nullptr;
  size_t size_ = 0;

 public:
  DevArray() = default;

  explicit DevArray(const size_t size) { Allocate(size); }

  DevArray(const DevArray&) = delete;

  DevArray& operator=(const DevArray&) = delete;

  ~DevArray() { cudaFree(ptr_); }

  void Allocate(const size_t size) {
    cudaFree(ptr_);
    ptr_ = nullptr;
    size_ = size;
    if (size > 0) {
      GCC_CUDA_CHECK(cudaMalloc(&ptr_, size * sizeof(T)));
    }
  }

  void Upload(const std::vector<T>& host) {
    GCC_CUDA_CHECK(cudaMemcpy(ptr_, host.data(), host.size() * sizeof(T), cudaMemcpyHostToDevice));
  }

  void Download(std::vector<T>& host) const {
    host.resize(size_);
    GCC_CUDA_CHECK(cudaMemcpy(host.data(), ptr_, size_ * sizeof(T), cudaMemcpyDeviceToHost));
  }

  T* Get() { return ptr_; }

  const T* Get() const { return ptr_; }

  [[nodiscard]] size_t Size() const { return size_; }
};

using Label = signed char;
using Rng = curandStatePhilox4_32_10_t;

constexpr unsigned kWarpSize = 32;
/**
 * Blocks are capped at 512 threads: Turing runs at most 1024 threads per
 * SM, so two blocks overlap and hide the barrier latency of the local
 * search steps. Measured on identical inputs: 512 beats 1024 by 10-45%
 * on NS2CC and is neutral on NS3CC.
 */
constexpr unsigned kMaxThreads = 512;
/** Default dynamic shared memory budget per block, bytes. */
constexpr unsigned kSharedBudget = 48 * 1024 - 256;

/**
 * Flatten the graph into a dense byte matrix with a zero diagonal.
 * Rows are filled by several host threads: IGraph::IsJoined is a virtual
 * call and the single-threaded loop took ~60 ms for 6000 vertices.
 */
inline std::vector<uint8_t> FlattenGraph(const IGraph& graph) {
  const unsigned n = graph.Size();
  std::vector<uint8_t> adj(static_cast<size_t>(n) * n, 0);
  const unsigned hw = std::thread::hardware_concurrency();
  const unsigned num_threads = std::clamp<unsigned>(hw == 0 ? 1 : hw, 1, std::max(1u, n / 64));
  auto fill_rows = [&](const unsigned first, const unsigned step) {
    for (unsigned i = first; i < n; i += step) {
      uint8_t* row = adj.data() + static_cast<size_t>(i) * n;
      for (unsigned j = 0; j < n; ++j) {
        row[j] = (j != i && graph.IsJoined(i, j)) ? 1 : 0;
      }
    }
  };
  if (num_threads == 1) {
    fill_rows(0, 1);
  } else {
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (unsigned t = 0; t < num_threads; ++t) {
      threads.emplace_back(fill_rows, t, num_threads);
    }
    for (auto& t : threads) {
      t.join();
    }
  }
  return adj;
}

/** Block size for a graph of n vertices: n rounded up to a warp, capped. */
inline unsigned ThreadsForSize(const unsigned n) {
  const unsigned rounded = ((n + kWarpSize - 1) / kWarpSize) * kWarpSize;
  return rounded < kWarpSize ? kWarpSize : (rounded > kMaxThreads ? kMaxThreads : rounded);
}

inline void CheckGraphSize(const unsigned n, const unsigned max_vertices, const char* algorithm) {
  if (n == 0 || n > max_vertices) {
    throw std::invalid_argument(std::string(algorithm) + " supports graphs with 1.." + std::to_string(max_vertices) +
                                " vertices, actual = " + std::to_string(n));
  }
}

inline unsigned long long RandomSeed() {
  std::random_device rd;
  return (static_cast<unsigned long long>(rd()) << 32) ^ rd();
}

inline bool HasCudaDevice() {
  int count = 0;
  return cudaGetDeviceCount(&count) == cudaSuccess && count > 0;
}

/**
 * Block-wide arg max; ties are broken towards the smallest index, which is
 * what the sequential CPU scans do.
 */
__device__ inline void BlockArgMax(int val, unsigned idx, int& out_val, unsigned& out_idx) {
  __shared__ int s_val[kWarpSize];
  __shared__ unsigned s_idx[kWarpSize];
  const unsigned lane = threadIdx.x & (kWarpSize - 1);
  const unsigned warp = threadIdx.x / kWarpSize;

  for (unsigned off = kWarpSize / 2; off > 0; off >>= 1) {
    const int ov = __shfl_down_sync(0xffffffffu, val, off);
    if (const unsigned oi = __shfl_down_sync(0xffffffffu, idx, off); ov > val || (ov == val && oi < idx)) {
      val = ov;
      idx = oi;
    }
  }
  if (lane == 0) {
    s_val[warp] = val;
    s_idx[warp] = idx;
  }
  __syncthreads();
  if (warp == 0) {
    const unsigned num_warps = blockDim.x / kWarpSize;
    val = lane < num_warps ? s_val[lane] : INT_MIN;
    idx = lane < num_warps ? s_idx[lane] : UINT_MAX;
    for (unsigned off = kWarpSize / 2; off > 0; off >>= 1) {
      const int ov = __shfl_down_sync(0xffffffffu, val, off);
      if (const unsigned oi = __shfl_down_sync(0xffffffffu, idx, off); ov > val || (ov == val && oi < idx)) {
        val = ov;
        idx = oi;
      }
    }
    if (lane == 0) {
      s_val[0] = val;
      s_idx[0] = idx;
    }
  }
  __syncthreads();
  out_val = s_val[0];
  out_idx = s_idx[0];
  __syncthreads();
}

__device__ inline long long BlockSum(long long val) {
  __shared__ long long s_sum[kWarpSize];
  const unsigned lane = threadIdx.x & (kWarpSize - 1);
  const unsigned warp = threadIdx.x / kWarpSize;

  for (unsigned off = kWarpSize / 2; off > 0; off >>= 1) {
    val += __shfl_down_sync(0xffffffffu, val, off);
  }
  if (lane == 0) {
    s_sum[warp] = val;
  }
  __syncthreads();
  if (warp == 0) {
    const unsigned num_warps = blockDim.x / kWarpSize;
    val = lane < num_warps ? s_sum[lane] : 0;
    for (unsigned off = kWarpSize / 2; off > 0; off >>= 1) {
      val += __shfl_down_sync(0xffffffffu, val, off);
    }
    if (lane == 0) {
      s_sum[0] = val;
    }
  }
  __syncthreads();
  const long long result = s_sum[0];
  __syncthreads();
  return result;
}

/** Initialize one Philox state per thread of every block. */
__global__ inline void InitRngKernel(Rng* rng, const unsigned long long seed) {
  const unsigned id = blockIdx.x * blockDim.x + threadIdx.x;
  curand_init(seed, id, 0, &rng[id]);
}

/**
 * Tournament selection, executed by thread 0 of the block with the block's
 * first RNG state. Returns the index of the chosen individual.
 */
__device__ inline unsigned Tournament(const unsigned* __restrict__ pop_dist, const unsigned population_size,
                                      const unsigned tournament_size, Rng& state) {
  unsigned best = curand(&state) % population_size;
  for (unsigned k = 1; k < tournament_size; ++k) {
    const unsigned cand = curand(&state) % population_size;
    if (pop_dist[cand] < pop_dist[best]) {
      best = cand;
    }
  }
  return best;
}

// -----------------------------------------------------------------------
// int8 GEMM on tensor cores (cuBLAS). Used to compute the O(n^2)
// "improvements"/"gains" of many clusterings at once: with the adjacency
// matrix as a +-1 (or 0/1) int8 matrix and the labels as int8 columns,
// one GEMM replaces one full pass over the matrix per clustering.
// cuBLAS int8 GEMM needs m and k to be multiples of 4, hence PadTo4.
// -----------------------------------------------------------------------

#define GCC_CUBLAS_CHECK(call)                                                                                \
  do {                                                                                                        \
    const cublasStatus_t st__ = (call);                                                                       \
    if (st__ != CUBLAS_STATUS_SUCCESS) {                                                                      \
      throw std::runtime_error(std::string("cuBLAS error ") + std::to_string(static_cast<int>(st__)) + " (" + \
                               std::string(__FILE__) + ":" + std::to_string(__LINE__) + ")");                 \
    }                                                                                                         \
  } while (0)

/** Leading dimension for padded int8 matrices. */
inline unsigned PadTo4(const unsigned n) { return (n + 3) & ~3u; }

/** Owning cuBLAS handle. */
class CublasHandle {
  cublasHandle_t handle_ = nullptr;

 public:
  CublasHandle() { GCC_CUBLAS_CHECK(cublasCreate(&handle_)); }

  CublasHandle(const CublasHandle&) = delete;

  CublasHandle& operator=(const CublasHandle&) = delete;

  ~CublasHandle() {
    if (handle_ != nullptr) {
      cublasDestroy(handle_);
    }
  }

  cublasHandle_t Get() const { return handle_; }
};

/**
 * C = A^T * B with int8 inputs and int32 output, all column-major:
 * A is k x m (lda = k), B is k x ncols (ldb = k), C is m x ncols (ldc = m).
 * m and k must be multiples of 4. Since the adjacency matrix is symmetric,
 * A^T * B is simply "adjacency times label columns".
 */
inline void GemmInt8(const CublasHandle& handle, const unsigned m, const unsigned ncols, const unsigned k,
                     const int8_t* a, const int8_t* b, int32_t* c) {
  if (ncols == 0) {
    return;
  }
  const int alpha = 1;
  const int beta = 0;
  GCC_CUBLAS_CHECK(cublasGemmEx(handle.Get(), CUBLAS_OP_T, CUBLAS_OP_N, static_cast<int>(m), static_cast<int>(ncols),
                                static_cast<int>(k), &alpha, a, CUDA_R_8I, static_cast<int>(k), b, CUDA_R_8I,
                                static_cast<int>(k), &beta, c, CUDA_R_32I, static_cast<int>(m), CUBLAS_COMPUTE_32I,
                                CUBLAS_GEMM_DEFAULT));
}

/**
 * Build a padded int8 copy of the 0/1 adjacency matrix (n x n, row-major
 * == column-major by symmetry) with leading dimension n_pad:
 * off-diagonal entries become `joined_value` / `not_joined_value`, the
 * diagonal becomes `diagonal_value`, padding is zero.
 * One thread per entry of the padded matrix.
 */
__global__ inline void BuildPaddedMatrixKernel(const uint8_t* __restrict__ adj, const unsigned n, const unsigned n_pad,
                                               const int8_t joined_value, const int8_t not_joined_value,
                                               const int8_t diagonal_value, int8_t* __restrict__ out) {
  const size_t idx = static_cast<size_t>(blockIdx.x) * blockDim.x + threadIdx.x;
  const size_t total = static_cast<size_t>(n_pad) * n_pad;
  if (idx >= total) {
    return;
  }
  const unsigned col = static_cast<unsigned>(idx / n_pad);
  const unsigned row = static_cast<unsigned>(idx % n_pad);
  int8_t value = 0;
  if (row < n && col < n) {
    if (row == col) {
      value = diagonal_value;
    } else {
      value = adj[static_cast<size_t>(col) * n + row] != 0 ? joined_value : not_joined_value;
    }
  }
  out[idx] = value;
}

/** Launch BuildPaddedMatrixKernel; `out` holds n_pad * n_pad entries. */
inline void BuildPaddedMatrix(const uint8_t* adj, const unsigned n, const unsigned n_pad, const int8_t joined_value,
                              const int8_t not_joined_value, const int8_t diagonal_value, int8_t* out) {
  const size_t total = static_cast<size_t>(n_pad) * n_pad;
  const unsigned threads = 256;
  const unsigned blocks = static_cast<unsigned>((total + threads - 1) / threads);
  BuildPaddedMatrixKernel<<<blocks, threads>>>(adj, n, n_pad, joined_value, not_joined_value, diagonal_value, out);
  GCC_CUDA_CHECK(cudaGetLastError());
}
}  // namespace gcc_cuda
