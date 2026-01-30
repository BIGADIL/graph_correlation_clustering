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

There are no tests or linter configured. Verify changes by building successfully.

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
- **pthreads** — for multi-threaded algorithm execution

### Key Conventions

- Headers in `include/`, implementations in `src/`, mirroring directory structure
- Namespaces match problem variants: `strict_2cc`, `non_strict_2cc`, `non_strict_3cc`, `semi_supervised_2cc`, `set_semi_supervised_2cc`
- Config parameters: `num_threads`, `num_graphs`, `graph_size[]`, `density[]`, `algorithms[]`, `parts[]` (semi-supervised only)
