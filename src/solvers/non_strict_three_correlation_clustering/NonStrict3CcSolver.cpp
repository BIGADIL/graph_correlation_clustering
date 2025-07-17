#include <algorithm>

#include "../../../include/solvers/non_strict_three_correlation_clustering/NonStrict3CcSolver.hpp"
#include "../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/NeighborhoodWithLocalSearch.hpp"
#include "../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhood.hpp"
#include "../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithLocalSearch.hpp"
#include "../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/BranchAndBounds.hpp"
#include "../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/BrutForce.hpp"
#include "../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithManyLocalSearches.hpp"
#include "../../../include/solvers/non_strict_three_correlation_clustering/genetic_algorithms/GeneticAlgorithm.hpp"

std::string non_strict_3cc::NonStrict3CCSolver::FormatComputationToJson(const IGraph &graph,
                                                                        const std::vector<ClusteringInfo> &computation_results,
                                                                        unsigned int size,
                                                                        double density) {
  std::stringstream ss;
  ss << "{ " << std::endl;
  ss << "\"size\": " << size << "," << std::endl;
  ss << "\"density\": " << density << "," << std::endl;
  ss << graph.ToJson() << "," << std::endl;
  unsigned idx = 0;
  for (const auto &computation_result: computation_results) {
    idx++;
    if (idx == computation_results.size()) {
      ss << computation_result.ToJson() << std::endl;
    } else {
      ss << computation_result.ToJson() << "," << std::endl;
    }
  }
  ss << "}" << std::endl;
  return ss.str();
}

non_strict_3cc::NonStrict3CCSolver::NonStrict3CCSolver(unsigned int num_threads,
                                                       IClustFactoryPtr factory) :
    num_threads_(num_threads),
    factory_(std::move(factory)) {

}

std::string non_strict_3cc::NonStrict3CCSolver::solve(const IGraphPtr &graph,
                                                      double density,
                                                      std::vector<std::string> used_algorithms) const {
  std::vector<ClusteringInfo> infos;
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "NeighborhoodWithLocalSearch")
      != used_algorithms.end()) {
    NeighborhoodWithLocalSearch nls(num_threads_, factory_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = nls.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "NeighborhoodWithLocalSearch",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "TwoVerticesNeighborhood")
      != used_algorithms.end()) {
    TwoVerticesNeighborhood twn(num_threads_, factory_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = twn.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "TwoVerticesNeighborhood",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "TwoVerticesNeighborhoodWithLocalSearch")
      != used_algorithms.end()) {
    TwoVerticesNeighborhoodWithLocalSearch twnls(num_threads_, factory_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = twnls.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "TwoVerticesNeighborhoodWithLocalSearch",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "TwoVerticesNeighborhoodWithManyLocalSearches")
      != used_algorithms.end()) {
    TwoVerticesNeighborhoodWithManyLocalSearches twnls(num_threads_, factory_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = twnls.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "TwoVerticesNeighborhoodWithManyLocalSearches",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "Genetic") != used_algorithms.end()) {
    GeneticAlgorithm genetic(1000, 100, factory_, 1024, 40, 1e-3);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = genetic.Train(graph);
    infos.emplace_back(
        "Genetic",
        clustering.clustering,
        clustering.distance,
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "BranchAndBounds") != used_algorithms.end()) {
    TwoVerticesNeighborhoodWithLocalSearch twnls(num_threads_, factory_);
    auto approximate_clustering = twnls.getBestNeighborhoodClustering(*graph);
    BranchAndBounds bb;
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = bb.GetBestClustering(graph, approximate_clustering);
    infos.emplace_back(
        "BranchAndBounds",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "BrutForce") != used_algorithms.end()) {
    BrutForce bf(factory_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = bf.GetBestClustering(graph);
    infos.emplace_back(
        "BrutForce",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  return FormatComputationToJson(*graph, infos, graph->Size(), density);
}
