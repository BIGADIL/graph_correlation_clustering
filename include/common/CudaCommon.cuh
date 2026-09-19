#pragma once

#include <cuda_runtime.h>
#include <curand_kernel.h>

#include <climits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "../graphs/IGraph.hpp"

namespace gcc_cuda {
#define GCC_CUDA_CHECK(call)                                                \
  do {                                                                      \
    const cudaError_t err__ = (call);                                       \
    if (err__ != cudaSuccess) {                                             \
      throw std::runtime_error(std::string("CUDA error: ") +                \
                               cudaGetErrorString(err__) + " (" +           \
                               std::string(__FILE__) + ":" +                \
                               std::to_string(__LINE__) + ")");             \
    }                                                                       \
  } while (0)

    /** Owning device array. */
    template<typename T>
    class DevArray {
        T *ptr_ = nullptr;
        size_t size_ = 0;

    public:
        DevArray() = default;

        explicit DevArray(const size_t size) { Allocate(size); }

        DevArray(const DevArray &) = delete;

        DevArray &operator=(const DevArray &) = delete;

        ~DevArray() { cudaFree(ptr_); }

        void Allocate(const size_t size) {
            cudaFree(ptr_);
            ptr_ = nullptr;
            size_ = size;
            if (size > 0) {
                GCC_CUDA_CHECK(cudaMalloc(&ptr_, size * sizeof(T)));
            }
        }

        void Upload(const std::vector<T> &host) {
            GCC_CUDA_CHECK(cudaMemcpy(ptr_, host.data(), host.size() * sizeof(T),
                cudaMemcpyHostToDevice));
        }

        void Download(std::vector<T> &host) const {
            host.resize(size_);
            GCC_CUDA_CHECK(cudaMemcpy(host.data(), ptr_, size_ * sizeof(T),
                cudaMemcpyDeviceToHost));
        }

        T *Get() { return ptr_; }

        const T *Get() const { return ptr_; }

        [[nodiscard]] size_t Size() const { return size_; }
    };

    using Label = signed char;
    using Rng = curandStatePhilox4_32_10_t;

    constexpr unsigned kWarpSize = 32;
    constexpr unsigned kMaxThreads = 1024;
    /** Default dynamic shared memory budget per block, bytes. */
    constexpr unsigned kSharedBudget = 48 * 1024 - 256;

    /** Flatten the graph into a dense byte matrix with a zero diagonal. */
    inline std::vector<uint8_t> FlattenGraph(const IGraph &graph) {
        const unsigned n = graph.Size();
        std::vector<uint8_t> adj(static_cast<size_t>(n) * n, 0);
        for (unsigned i = 0; i < n; ++i) {
            for (unsigned j = i + 1; j < n; ++j) {
                const uint8_t joined = graph.IsJoined(i, j) ? 1 : 0;
                adj[static_cast<size_t>(i) * n + j] = joined;
                adj[static_cast<size_t>(j) * n + i] = joined;
            }
        }
        return adj;
    }

    /** Block size for a graph of n vertices: n rounded up to a warp, capped. */
    inline unsigned ThreadsForSize(const unsigned n) {
        const unsigned rounded = ((n + kWarpSize - 1) / kWarpSize) * kWarpSize;
        return rounded < kWarpSize
                   ? kWarpSize
                   : (rounded > kMaxThreads ? kMaxThreads : rounded);
    }

    inline void CheckGraphSize(const unsigned n, const unsigned max_vertices,
                               const char *algorithm) {
        if (n == 0 || n > max_vertices) {
            throw std::invalid_argument(
                std::string(algorithm) + " supports graphs with 1.." +
                std::to_string(max_vertices) + " vertices, actual = " +
                std::to_string(n));
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
    __device__ inline void BlockArgMax(int val, unsigned idx, int &out_val,
                                       unsigned &out_idx) {
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
    __global__ inline void InitRngKernel(Rng *rng, const unsigned long long seed) {
        const unsigned id = blockIdx.x * blockDim.x + threadIdx.x;
        curand_init(seed, id, 0, &rng[id]);
    }

    /**
     * Tournament selection, executed by thread 0 of the block with the block's
     * first RNG state. Returns the index of the chosen individual.
     */
    __device__ inline unsigned Tournament(const unsigned *__restrict__ pop_dist,
                                          const unsigned population_size,
                                          const unsigned tournament_size,
                                          Rng &state) {
        unsigned best = curand(&state) % population_size;
        for (unsigned k = 1; k < tournament_size; ++k) {
            const unsigned cand = curand(&state) % population_size;
            if (pop_dist[cand] < pop_dist[best]) {
                best = cand;
            }
        }
        return best;
    }
} // namespace gcc_cuda
