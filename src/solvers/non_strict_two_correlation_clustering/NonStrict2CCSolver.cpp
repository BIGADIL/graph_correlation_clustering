#include <algorithm>
#include <utility>

#include "../../../include/solvers/non_strict_two_correlation_clustering/NonStrict2CCSolver.hpp"
#include "../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithManyLocalSearches.hpp"
#include "../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithOneLocalSearch.hpp"
#include "../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/Neighborhood.hpp"
#include "../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/BranchAndBounds.hpp"
#include "../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/BrutForce.hpp"
#include "../../../include/solvers/non_strict_two_correlation_clustering/ipls_algorithms/IPLSAlgorithm.hpp"

std::string non_strict_2cc::NonStrict2CCSolver::solve(const IGraphPtr &graph,
                                                      const std::string& distribution,
                                                      double density,
                                                      std::vector<std::string> used_algorithms) const {
  std::vector<ClusteringInfo> infos;
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "NeighborhoodWithManyLocalSearches")
      != used_algorithms.end()) {
    NeighborhoodWithManyLocalSearches nmls(num_threads_, factory_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = nmls.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "NeighborhoodWithManyLocalSearches",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "NeighborhoodWithOneLocalSearch")
      != used_algorithms.end()) {
    NeighborhoodWithOneLocalSearch nols(num_threads_, factory_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = nols.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "NeighborhoodWithOneLocalSearch",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "Neighborhood") != used_algorithms.end()) {
    Neighborhood n(num_threads_, factory_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = n.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "Neighborhood",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "BranchAndBounds") != used_algorithms.end()) {
    IPLSAlgorithm genetic(100, 6, factory_, 128, 5, 0.4, num_threads_);
    auto sol = genetic.Train(graph);
    BranchAndBounds bb;
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = bb.GetBestClustering(graph, sol.clustering);
    infos.emplace_back(
        "BranchAndBounds",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "Genetic") != used_algorithms.end()) {
    IPLSAlgorithm genetic(100, 6, factory_, 128, 5, 0.4, num_threads_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = genetic.Train(graph);
    infos.emplace_back(
        "Genetic",
        clustering.clustering,
        clustering.distance,
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
  return FormatComputationToJson(*graph, infos, graph->Size(), density, distribution);
}

std::string non_strict_2cc::NonStrict2CCSolver::solve(const IGraphPtr &graph,
                                                      double density,
                                                      std::vector<std::string> used_algorithms) const {
  return solve(graph, "", density, std::move(used_algorithms));
}

std::string non_strict_2cc::NonStrict2CCSolver::solve(const IGraphPtr &graph,
                                                      const std::string& distribution,
                                                      std::vector<std::string> used_algorithms) const {
  return solve(graph, distribution, -1, std::move(used_algorithms));
}

non_strict_2cc::NonStrict2CCSolver::NonStrict2CCSolver(const unsigned num_threads,
                                                       IClustFactoryPtr factory) :
    num_threads_(num_threads),
    factory_(std::move(factory)) {

}

std::string non_strict_2cc::NonStrict2CCSolver::FormatComputationToJson(const IGraph &graph,
                                                                        const std::vector<ClusteringInfo> &computation_results,
                                                                        unsigned int size,
                                                                        double density,
                                                                        const std::string& distribution) {
  std::stringstream ss;
  ss << "{ " << std::endl;
  ss << "\"size\": " << size << "," << std::endl;
  if (density != -1) {
    ss << "\"density\": " << density << "," << std::endl;
  }
  if (!distribution.empty()) {
    ss << "\"distribution\": " << "\"" << distribution << "\"," << std::endl;
  }
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
