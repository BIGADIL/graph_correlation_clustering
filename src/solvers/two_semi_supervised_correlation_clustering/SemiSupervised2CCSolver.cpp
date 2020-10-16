#include <algorithm>

#include "../../../include/solvers/two_semi_supervised_correlation_clustering/SemiSupervised2CCSolver.hpp"
#include "../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.hpp"
#include "../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.hpp"
#include "../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/Neighborhood.hpp"
#include "../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BranchAndBounds.hpp"
#include "../../../include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BrutForce.hpp"

std::string semi_supervised_2cc::SemiSupervised2CCSolver::solve(const IGraphPtr &graph,
                                                                const double density,
                                                                std::vector<std::string> used_algorithms,
                                                                unsigned first_cluster_vertex,
                                                                unsigned second_cluster_vertex) const {
  std::vector<ClusteringInfo> infos;
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "NeighborhoodWithManyLocalSearches")
      != used_algorithms.end()) {
    NeighborhoodWithManyLocalSearches nmls(num_threads_, factory_);
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = nmls.getBestNeighborhoodClustering(
        *graph,
        first_cluster_vertex,
        second_cluster_vertex);
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
    auto clustering = nols.getBestNeighborhoodClustering(
        *graph,
        first_cluster_vertex,
        second_cluster_vertex);
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
    auto clustering = n.getBestNeighborhoodClustering(
        *graph,
        first_cluster_vertex,
        second_cluster_vertex);
    infos.emplace_back(
        "Neighborhood",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "BranchAndBounds") != used_algorithms.end()) {
    NeighborhoodWithManyLocalSearches nmls(num_threads_, factory_);
    auto approximate_clustering = nmls.getBestNeighborhoodClustering(
        *graph,
        first_cluster_vertex,
        second_cluster_vertex);
    BranchAndBounds bb;
    auto start_time = std::chrono::steady_clock::now();
    auto clustering = bb.GetBestClustering(
        graph,
        approximate_clustering,
        first_cluster_vertex,
        second_cluster_vertex);
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
    auto clustering = bf.GetBestClustering(graph, first_cluster_vertex, second_cluster_vertex);
    infos.emplace_back(
        "BrutForce",
        clustering,
        clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time)
    );
  }
  return FormatComputationToJson(
      *graph,
      infos,
      graph->Size(),
      density,
      first_cluster_vertex,
      second_cluster_vertex);
}

semi_supervised_2cc::SemiSupervised2CCSolver::SemiSupervised2CCSolver(const unsigned num_threads,
                                                                      IClustFactoryPtr factory) :
    num_threads_(num_threads),
    factory_(std::move(factory)) {

}

std::string semi_supervised_2cc::SemiSupervised2CCSolver::FormatComputationToJson(const IGraph &graph,
                                                                                  const std::vector<ClusteringInfo> &computation_results,
                                                                                  const unsigned int size,
                                                                                  const double density,
                                                                                  unsigned first_cluster_vertex,
                                                                                  unsigned second_cluster_vertex) {
  std::stringstream ss;
  ss << "{ " << std::endl;
  ss << "\"size\": " << size << "," << std::endl;
  ss << "\"density\": " << density << "," << std::endl;
  ss << "\"clustered_vertices\": [" << first_cluster_vertex << ", " << second_cluster_vertex << "], " << std::endl;
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
