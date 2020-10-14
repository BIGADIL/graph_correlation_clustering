#include <algorithm>

#include "../../../include/solvers/strict_two_correlation_clustering/S2CCSolver.hpp"
#include "../../../include/solvers/strict_two_correlation_clustering/clust_algorithms/NNLSAlgorithmForS2CC.hpp"
#include "../../../include/solvers/strict_two_correlation_clustering/clust_algorithms/N1LSAlgorithmForS2CC.hpp"
#include "../../../include/solvers/strict_two_correlation_clustering/clust_algorithms/NAlgorithmForS2CС.hpp"
#include "../../../include/solvers/strict_two_correlation_clustering/clust_algorithms/BBAlgorithmForS2CC.hpp"

std::string S2CCSolver::solve(const IGraphPtr &graph,
                              const double density,
                              std::vector<std::string> used_algorithms) const {
  std::vector<ClusteringInfo> infos;
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "NNLS") != used_algorithms.end()) {
    NNLSAlgorithmForS2CC nnls_algoritm(num_threads_, factory_);
    auto nnls_start_time = std::chrono::steady_clock::now();
    auto nnls_clustering = nnls_algoritm.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "NNLS",
        nnls_clustering,
        nnls_clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - nnls_start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "N1LS") != used_algorithms.end()) {
    N1LSAlgorithmForS2CC n1ls_algorithm(num_threads_, factory_);
    auto n1ls_start_time = std::chrono::steady_clock::now();
    auto nn1s_clustering = n1ls_algorithm.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "N1LS",
        nn1s_clustering,
        nn1s_clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - n1ls_start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "N") != used_algorithms.end()) {
    NAlgorithmForS2CC n_algorithm(num_threads_, factory_);
    auto n_start_time = std::chrono::steady_clock::now();
    auto n_clustering = n_algorithm.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "N",
        n_clustering,
        n_clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - n_start_time)
    );
  }
  if (std::find(used_algorithms.begin(), used_algorithms.end(), "BB") != used_algorithms.end()) {
    NNLSAlgorithmForS2CC nnls_algoritm(num_threads_, factory_);
    auto nnls_clustering = nnls_algoritm.getBestNeighborhoodClustering(*graph);
    BBAlgorithmForS2CC bb_algorithm;
    auto bb_start_time = std::chrono::steady_clock::now();
    auto bbm_record = bb_algorithm.GetBestClustering(graph, nnls_clustering);
    infos.emplace_back(
        "BB",
        bbm_record,
        bbm_record->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - bb_start_time)
    );
  }
  return FormatComputationToJson(*graph, infos, graph->Size(), density);
}

S2CCSolver::S2CCSolver(const unsigned num_threads,
                       IClustFactoryPtr factory) :
    num_threads_(num_threads),
    factory_(std::move(factory)) {

}

std::string S2CCSolver::FormatComputationToJson(const IGraph &graph,
                                                const std::vector<ClusteringInfo> &computation_results,
                                                const unsigned int size,
                                                const double density) {
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
