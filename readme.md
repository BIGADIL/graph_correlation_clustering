# Graph Correlation Clustering

This project contains implementation of algorithms for graph correlation clustering problems.

## Overview

The project implements various algorithms for solving correlation clustering problems on graphs, including strict, non-strict, and semi-supervised variants. Algorithms support both 2-correlation and 3-correlation clustering.

## Correlation Clustering Problem

### Problem Definition

**Correlation clustering** is a graph clustering problem where the goal is to partition graph vertices into clusters based on pairwise similarity/dissimilarity information encoded in the graph edges.

Given a graph G = (V, E) where:
- V is the set of vertices
- E is the set of edges (representing similarity between vertices)

The task is to find a clustering (partition) C of vertices V into k clusters that minimizes the **objective function** (distance between the graph and clustering):

**Distance = Number of disagreements**, where disagreement occurs when:
- Two vertices are in the **same cluster** but there is **NO edge** between them (false positive)
- Two vertices are in **different clusters** but there **IS an edge** between them (false negative)

### Problem Variants

#### 1. Strict k-Correlation Clustering
- **All vertices must be assigned** to exactly one of k clusters
- No vertices can remain unclustered
- k ∈ {2, 3} - partition into 2 or 3 clusters

#### 2. Non-Strict k-Correlation Clustering
- **All vertices must be assigned** to exactly one of k clusters
- No vertices can remain unclustered
- k ∈ {2, 3} clusters

#### 3. Semi-Supervised k-Correlation Clustering
- A subset of vertices is **pre-clustered** (fixed labels)
- The algorithm must cluster the remaining vertices while respecting the fixed assignments
- Useful when partial ground truth or domain knowledge is available
- Supports must-link and cannot-link constraints

#### 4. Set Semi-Supervised k-Correlation Clustering
- Similar to semi-supervised, but with sets of pre-clustered vertices
- Multiple sets of vertices with known cluster assignments

### Applications

Correlation clustering has numerous real-world applications:

- **Social Network Analysis**: Community detection, friend recommendation
- **Document Clustering**: Grouping similar documents based on pairwise similarity
- **Image Segmentation**: Partitioning images into regions based on pixel similarity
- **Bioinformatics**: Protein interaction networks, gene clustering
- **Recommendation Systems**: User/item grouping
- **Data Deduplication**: Identifying and grouping duplicate records

### Computational Complexity

The correlation clustering problem is **NP-hard**, meaning there is no known polynomial-time algorithm that guarantees optimal solutions for arbitrary inputs. This project implements several approaches:

1. **Exact algorithms**: Branch-and-Bounds, Brute Force (for small graphs)
2. **Heuristic algorithms**: Neighborhood-based methods, Local Search
3. **Meta-heuristics**: IPLS (Iterated Population Local Search)

## Implemented Algorithms

### Algorithm Types

#### Exact Algorithms
- **BrutForce**: Exhaustive search through all possible clusterings. Guarantees optimal solution but only practical for very small graphs (n < 15)
- **BranchAndBounds**: Uses pruning strategies to reduce the search space while still guaranteeing optimal solutions. More efficient than brute force but still limited to small-medium graphs

#### Greedy Heuristics
- **Neighborhood**: Explores clustering by splitting vertices based on neighborhood structure. For each vertex i creates a clustering where vertex i's neighbors are placed in one cluster and non-neighbors in another
- **NeighborhoodWithOneLocalSearch**: Neighborhood algorithm enhanced with a single local search refinement pass
- **NeighborhoodWithManyLocalSearches**: Multiple iterations of local search from different neighborhood-based starting points

#### Advanced Heuristics
- **IPLS (Iterated Population Local Search)**: Meta-heuristic that iteratively applies partial destruction and reconstruction of solutions combined with local search. Effective for larger graphs
- **NeighborhoodOfPreClusteringVertices**: (Semi-supervised only) Specialized algorithm that exploits pre-clustered vertices to guide the clustering process

#### Specialized for 3-Correlation
- **TwoVerticesNeighborhood**: Considers pairs of vertices for 3-way clustering
- **TwoVerticesNeighborhoodWithLocalSearch**: Enhanced with local search
- **TwoVerticesNeighborhoodWithManyLocalSearches**: Multiple local search iterations

### Implemented Solvers by Problem Type

#### Strict 2-Correlation Clustering
- Neighborhood
- NeighborhoodWithOneLocalSearch
- NeighborhoodWithManyLocalSearches
- BranchAndBounds
- BrutForce

#### Non-Strict 2-Correlation Clustering
- Neighborhood
- NeighborhoodWithOneLocalSearch
- NeighborhoodWithManyLocalSearches
- BranchAndBounds
- BrutForce
- IPLS

#### Non-Strict 3-Correlation Clustering
- TwoVerticesNeighborhood
- TwoVerticesNeighborhoodWithLocalSearch
- TwoVerticesNeighborhoodWithManyLocalSearches
- NeighborhoodWithLocalSearch
- BranchAndBounds
- BrutForce
- IPLS

#### Semi-Supervised 2-Correlation Clustering
- Neighborhood
- NeighborhoodWithOneLocalSearch
- NeighborhoodWithManyLocalSearches
- NeighborhoodOfPreClusteringVertices
- BranchAndBounds
- BrutForce

#### Set Semi-Supervised 2-Correlation Clustering
- Neighborhood
- NeighborhoodWithOneLocalSearch
- NeighborhoodWithManyLocalSearches
- NeighborhoodOfPreClusteringVertices
- BranchAndBounds
- BrutForce

### Performance Characteristics

| Algorithm Type | Time Complexity | Solution Quality | Best For  |
|---------------|----------------|------------------|-----------|
| BrutForce | O(k^n) | Optimal | n < 15    |
| BranchAndBounds | Exponential (with pruning) | Optimal | n < 100   |
| Neighborhood | O(n^3) | Good approximation | n < 10000 |
| With Local Search | O(n^4) | Better approximation | n < 5000  |
| IPLS | Depends on iterations | High-quality | n > 10000 |

*Note: k = number of clusters, n = number of vertices*

## Requirements

- CMake 3.8 or higher
- C++20 compatible compiler
- pthreads

## Dependencies

- RapidJSON (included as external project)

## Build

```bash
# Create build directory
mkdir cmake-build-release
cd cmake-build-release

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build .
```

For debug build:
```bash
mkdir cmake-build-debug
cd cmake-build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## Executables

The project builds the following experiment executables:

- `Strict2CCExperiment` - Strict 2-correlation clustering experiments
- `NonStrict2CCExperiment` - Non-strict 2-correlation clustering experiments
- `NonStrict2CCExperimentSO` - Non-strict 2-correlation clustering single-objective experiments
- `NonStrict3CCExperiment` - Non-strict 3-correlation clustering experiments
- `NonStrict3CCExperimentSO` - Non-strict 3-correlation clustering single-objective experiments
- `SemiSupervised2CCExperiment` - Semi-supervised 2-correlation clustering experiments
- `SetSemiSupervised2CCExperiment` - Set semi-supervised 2-correlation clustering experiments

## Usage

Each experiment executable accepts a configuration file as a command-line argument:

```bash
./Strict2CCExperiment data/s2cc_config.json
```

### Configuration File Format

Configuration files are in JSON format. Example:

```json
{
  "num_threads": 8,
  "num_graphs": 14,
  "graph_size": [600],
  "density": [0.67],
  "algorithms": [
    "NeighborhoodWithManyLocalSearches",
    "NeighborhoodWithOneLocalSearch",
    "Neighborhood"
  ]
}
```

**Parameters:**
- `num_threads` - Number of threads for parallel execution
- `num_graphs` - Number of random graphs to generate and test
- `graph_size` - Array of graph sizes to test
- `density` - Array of graph densities (for Erdos-Renyi random graphs)
- `algorithms` - Array of algorithm names to run
- `parts` - (Semi-supervised only) Array of fractions of pre-clustered vertices

Example configuration files are available in the `data/` directory:
- `s2cc_config.json.example`
- `ns2cc_config.json.example`
- `ns3cc_config.json.example`
- `2scc_config.json.example`
- `2sscc_config.json.example`

### Output

Results are saved as JSON files in subdirectories created by each experiment:
- `strict_2cc/` - for Strict2CCExperiment
- `non_strict_2cc/` - for NonStrict2CCExperiment
- `non_strict_3cc/` - for NonStrict3CCExperiment
- `semi_supervised_2cc/` - for SemiSupervised2CCExperiment
- `set_semi_supervised_2cc/` - for SetSemiSupervised2CCExperiment

Results are organized by graph parameters: `n-{size}-p-{density}/`

#### Output Format

Each JSON file contains:
```json
{
  "AlgorithmName": {
    "binary clustering vector": [0, 1, 0, 1, ...],
    "objective function value": 42,
    "computation time seconds": 123
  },
  ...
}
```

**Fields:**
- `binary clustering vector` / `triple clustering vector`: Cluster assignment for each vertex (0, 1, or 2)
- `objective function value`: The distance (number of disagreements) between the clustering and the graph
- `computation time seconds`: Time taken by the algorithm in seconds

## Features

### Parallel Execution
All solvers support multi-threaded execution to leverage modern multi-core processors. The number of threads is configurable via the `num_threads` parameter.

### Graph Generation
- **Erdos-Renyi Random Graphs**: Generate random graphs with specified vertex count and edge probability
- **Tag-based Graphs**: Load real-world graphs from JSON files (see `Tags_nus_wide.json`, `Tags_so.json`)

### Experimental Framework
The project provides a complete experimental framework for:
- Running multiple algorithms on the same graph instances
- Testing across different graph sizes and densities
- Comparing algorithm performance and solution quality
- Generating reproducible experimental results

## Research Applications

This implementation is suitable for:
- **Benchmarking**: Comparing different correlation clustering algorithms
- **Algorithm Development**: Testing new heuristics and optimizations
- **Computational Studies**: Analyzing algorithm behavior on different graph types
- **Real-world Applications**: Solving practical clustering problems in various domains

## Project Structure

```
├── include/               # Header files
│   ├── clustering/        # Clustering representations and factories
│   │   ├── BinaryClusteringVector.hpp
│   │   ├── TripleClusteringVector.hpp
│   │   ├── BBBinaryClusteringVector.hpp
│   │   └── factories/     # Factory patterns for clustering creation
│   ├── graphs/            # Graph representations and factories
│   │   ├── IGraph.hpp
│   │   ├── AdjacencyMatrixGraph.hpp
│   │   └── factories/     # Erdos-Renyi, Tag-based graph factories
│   ├── solvers/           # Solver implementations
│   │   ├── strict_two_correlation_clustering/
│   │   ├── non_strict_two_correlation_clustering/
│   │   ├── non_strict_three_correlation_clustering/
│   │   ├── two_semi_supervised_correlation_clustering/
│   │   └── two_set_semi_supervised_correlation_clustering/
│   └── common/            # Common utilities
│       ├── ClusteringLabels.hpp
│       └── ExperimentParameters.hpp
├── src/                   # Implementation files (mirrors include/)
├── data/                  # Configuration files and test data
│   ├── *_config.json.example
│   ├── Tags_nus_wide.json
│   └── Tags_so.json
├── vendor/                # Third-party dependencies (RapidJSON)
├── Strict2CCExperiment.cpp
├── NonStrict2CCExperiment.cpp
├── NonStrict3CCExperiment.cpp
├── SemiSupervised2CCExperiment.cpp
├── SetSemiSupervised2CCExperiment.cpp
└── CMakeLists.txt
```

## Example Workflow

### 1. Prepare Configuration

Create a configuration file based on examples in `data/`:

```bash
cp data/s2cc_config.json.example my_experiment.json
```

Edit parameters as needed:
```json
{
  "num_threads": 4,
  "num_graphs": 10,
  "graph_size": [50, 100, 200],
  "density": [0.3, 0.5, 0.7],
  "algorithms": ["Neighborhood", "NeighborhoodWithOneLocalSearch"]
}
```

### 2. Run Experiment

```bash
./cmake-build-release/Strict2CCExperiment my_experiment.json
```

This will:
- Generate 10 random graphs for each combination of size and density
- Run selected algorithms on each graph
- Save results to `strict_2cc/n-{size}-p-{density}/` directories

### 3. Analyze Results

Results are in JSON format and can be analyzed with any JSON-compatible tools:

```bash
# View results
cat strict_2cc/n-100-p-0.5/n-100-p-0.5-*.json | jq .

# Compare objective function values
grep "objective function value" strict_2cc/n-100-p-0.5/*.json
```

## Tips and Best Practices

### Algorithm Selection

- **Small graphs (n < 50)**: Use `BranchAndBounds` or `BrutForce` for optimal solutions
- **Medium and large graphs (n > 50)**: Use `IPLS` or simple `Neighborhood` with reduced local search
- **Multiple objectives**: Run several algorithms and compare results

### Performance Tuning

- Adjust `num_threads` based on your CPU cores (typically 4-8 works well)
- For large graphs, reduce the number of local search iterations
- Use Release build (`-DCMAKE_BUILD_TYPE=Release`) for 5-10x speedup

### Graph Density Considerations

- **Low density (p < 0.3)**: Sparse graphs, clustering is generally easier
- **Medium density (0.3 < p < 0.7)**: Most challenging, algorithms show largest differences
- **High density (p > 0.7)**: Dense graphs, often easier to cluster

## License

This project is provided for research and educational purposes.

## Contributing

For bug reports or feature requests, please use the issue tracker.

## References

For more information on correlation clustering theory and algorithms:
- Bansal, N., Blum, A., & Chawla, S. (2004). "Correlation Clustering". Machine Learning, 56(1-3).
- Ailon, N., Charikar, M., & Newman, A. (2008). "Aggregating inconsistent information: Ranking and clustering". Journal of the ACM.
- Coulman, J., Saunderson, J., & Wirth, A. "Algorithms for Correlation Clustering".
- Giotis, I., & Guruswami, V. (2006). "Correlation Clustering with a Fixed Number of Clusters". Theory of Computing, 2(1).
- Ben-Dor, A., Shamir, R., & Yakhini, Z. (1999). "Clustering Gene Expression Patterns". Journal of Computational Biology, 6(3-4).
- Shamir, R., Sharan, R., & Tsur, D. (2004). "Cluster Graph Modification Problems". Discrete Applied Mathematics, 144(1-2).
- Il'ev, V., Il'eva, S., & Morshinin, A. "Approximate Algorithms for Graph Clustering Problem". PRIKLADNAYA DISKRETNAYA MATEMATIKA, 45.
- Il'ev, V., Il'eva, S., & Morshinin, A. (2020). "2-Approximation Algorithms for Two Graph Clustering Problems". Journal of Applied and Industrial Mathematics, 14(3).
- Il'ev, V., Il'eva, S., & Morshinin, A. (2020). "A 2-Approximation Algorithm for the Graph 2-Clustering Problem". LNCS, 11548.
- Il'ev, V., Il'eva, S., & Morshinin, A. (2020). "An Approximation Algorithm for a Semi-supervised Graph Clustering Problem". CCIS, 1275.
- Morshinin, A. "Experimental Study of Semi-Supervised Graph 2-Clustering Problem". Journal of Mathematical Sciences, 275(1).