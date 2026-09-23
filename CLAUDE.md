# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Release build
mkdir cmake-build-release && cd cmake-build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Debug build
mkdir cmake-build-debug && cd cmake-build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

# Build a single target
cmake --build cmake-build-release --target NonStrict2CCExperiment
```

CUDA is optional: if `nvcc` is found, CMake enables it, compiles `*.cu` sources and defines `GCC_HAS_CUDA`; pass `-DGCC_ENABLE_CUDA=OFF` to build without it. GCC 15 needs `-Wno-template-body` for the pinned RapidJSON revision (added automatically).

There are no tests or linter configured. Verify changes by building successfully. For GPU code, compare against the CPU implementation (`IPLSCudaAlgorithm::ComputeLocalOptimum` must reproduce the variant's `LocalSearch::ComputeLocalOptimum` bit-exactly).

## Running Experiments

Each executable takes a JSON config file as its argument:
```bash
./cmake-build-release/Strict2CCExperiment data/s2cc_config.json
```

Example configs are in `data/*.json.example`. Results are written as JSON to subdirectories like `strict_2cc/n-{size}-p-{density}/`.

## Architecture

This is a C++20 research project implementing algorithms for **graph correlation clustering** — partitioning graph vertices into k clusters to minimize disagreements (same-cluster vertices without edges + different-cluster vertices with edges).

### Problem Variants (5 solver families)

Each variant lives under `include/solvers/` and `src/solvers/` in its own directory with the same internal structure:

| Directory | Solver class | Description |
|-----------|-------------|-------------|
| `strict_two_correlation_clustering/` | `Strict2CCSolver` | All vertices must be in exactly 2 clusters |
| `non_strict_two_correlation_clustering/` | `NonStrict2CCSolver` | 2-cluster with non-strict objective |
| `non_strict_three_correlation_clustering/` | `NonStrict3CcSolver` | 3-cluster variant |
| `two_semi_supervised_correlation_clustering/` | `SemiSupervised2CCSolver` | Some vertices have fixed labels |
| `two_set_semi_supervised_correlation_clustering/` | `SetSemiSupervised2CCSolver` | Sets of vertices with fixed labels |

Each solver directory contains:
- `clust_algorithms/` (note: some header dirs use the typo `clust_algoritms/`) — algorithm implementations
- `common_functions/` — shared `NeighborSplitter` and `LocalSearch` for that variant
- A top-level solver class that dispatches by algorithm name string from config

### Algorithm Hierarchy

Algorithms are duplicated per problem variant (not shared across variants). Each variant implements its own versions of:

- **BrutForce** — exhaustive O(k^n), for n < 15
- **BranchAndBounds** — pruned exact search, for n < 100. Uses `BB*ClusteringVector` classes with precomputed bounds
- **Neighborhood** — O(n^3) greedy heuristic based on vertex neighborhood splitting
- **NeighborhoodWithOneLocalSearch / NeighborhoodWithManyLocalSearches** — neighborhood + local search refinement
- **IPLS** (non-strict 2CC and 3CC only) — population-based meta-heuristic with multi-threaded workers, barrier synchronization, and tournament selection
- **NeighborhoodCuda / NeighborhoodWithManyLocalSearchesCuda** (non-strict 2CC only, same algorithm names in config) — GPU versions of Neighborhood and NeighborhoodWithManyLocalSearches: thin classes in `clust_algoritms/*Cuda.hpp` over `common_functions/CudaNeighborhood.{hpp,cu}`, one CUDA block per split vertex. Device building blocks shared with IPLS (improvements, local search, neighborhood split) are in `common_functions/CudaLocalSearch.cuh`. Results equal the CPU versions; ties between vertices go to the smallest index.
- **TwoVerticesNeighborhoodCuda / TwoVerticesNeighborhoodWithManyLocalSearchesCuda** (non-strict 3CC only, same algorithm names in config) — GPU versions of the TwoVertices algorithms: thin classes in `clust_algorithms/*Cuda.hpp` over `common_functions/CudaTwoVerticesNeighborhood.{hpp,cu}`, one CUDA block per ordered pair of vertices, pairs processed in chunks. 3CC device building blocks shared with IPLS (gains, local search, splits) are in `common_functions/CudaLocalSearch.cuh`. Results equal the CPU versions; ties go to the lexicographically smallest pair.
- **IPLSCudaAlgorithm** (non-strict 2CC and 3CC, algorithm name `GeneticCuda`) — CUDA port of IPLS in `ipls_algorithms/IPLSCudaAlgorithm.cu` of each variant: one block per individual, threads split vertices; population stays in GPU memory. Same parameters as `IPLSAlgorithm` minus `num_threads`. CPU IPLS is dispatched as `Genetic`. Shared device/host helpers (device arrays, block reductions, RNG, tournament, multi-threaded graph flattening) live in `include/common/CudaCommon.cuh`; blocks are capped at 512 threads (`kMaxThreads`, measured better than 1024 on Turing). Local search steps fuse the gain update with the scan for the next candidate (one pass per step). The 3CC version keeps per-vertex move gains for all 3 labels and updates them incrementally in both local search and perturbation.

3CC additionally has `TwoVerticesNeighborhood*` variants that consider vertex pairs. Semi-supervised variants add `NeighborhoodOfPreClusteringVertices`.

### Core Data Model

- **`IGraph`** / **`AdjacencyMatrixGraph`** — `vector<vector<bool>>` adjacency matrix, O(1) edge lookup via `IsJoined(i, j)`
- **`IClustering`** / **`BinaryClusteringVector`** / **`TripleClusteringVector`** — label arrays where each vertex gets `FIRST_CLUSTER(0)`, `SECOND_CLUSTER(1)`, `THIRD_CLUSTER(2)`, or `NON_CLUSTERED(-1)`
- **`BB*ClusteringVector`** — branch-and-bounds optimized variants that track bounds incrementally
- **Graph factories**: `ErdosRenyiRandomGraphFactory` (random with density parameter), `TagsGraphFactory` (loads real-world data from `data/Tags_*.json`)

### Experiment Entry Points

Top-level `.cpp` files (e.g., `NonStrict2CCExperiment.cpp`) are the executables. They read config via `ExperimentParameters`, generate graphs, run selected algorithms, and write JSON results. Multi-objective (`*Experiment.cpp`) vs single-objective (`*ExperimentSO.cpp`) variants exist for non-strict problems.

### Dependencies

- **RapidJSON** — fetched via CMake ExternalProject at build time (header-only JSON parser)
- **cuBLAS** (only when CUDA is enabled) — int8 tensor-core GEMM (`gcc_cuda::GemmInt8` in `include/common/CudaCommon.cuh`) computes the O(n^2) improvements/gains of many clusterings at once: adjacency as a padded +-1 (2CC) or 0/1 (3CC) int8 matrix times int8 label columns. Used by IPLS 2CC (population init and after perturbation), Neighborhood 2CC (`B * (B + I)` gives all splits) and TwoVertices 3CC (indicator columns per pair). cuBLAS int8 needs m, k multiples of 4, hence `PadTo4`.
- **pthreads** — for multi-threaded algorithm execution

### Key Conventions

- Headers in `include/`, implementations in `src/`, mirroring directory structure
- Namespaces match problem variants: `strict_2cc`, `non_strict_2cc`, `non_strict_3cc`, `semi_supervised_2cc`, `set_semi_supervised_2cc`
- Config parameters: `num_threads`, `num_graphs`, `graph_size[]`, `density[]`, `algorithms[]`, `parts[]` (semi-supervised only)
