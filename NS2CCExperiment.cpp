#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>

#include "include/algorithms/non_strict_two_correlation_clustering/NNLSAlgorithmForNS2CC.hpp"
#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/clustering/factories/BinaryClusteringFactory.hpp"
#include "include/algorithms/non_strict_two_correlation_clustering/BBAlgorithmForNS2CC.hpp"
#include "include/algorithms/non_strict_two_correlation_clustering/N1LSAlgorithmForNS2CC.hpp"
#include "include/algorithms/non_strict_two_correlation_clustering/NAlgorithmForNS2CC.hpp"

struct ClusteringInfo {
  std::string name;
  std::shared_ptr<IClustering> clustering;
  unsigned objective_function_value;
  std::chrono::seconds computation_time;

  ClusteringInfo(std::string name,
                 std::shared_ptr<IClustering> clustering,
                 unsigned int objective_function_value,
                 std::chrono::seconds computation_time)
      : name(std::move(name)),
        clustering(std::move(clustering)),
        objective_function_value(objective_function_value),
        computation_time(computation_time) {}

  std::string ToJson() const {
    std::stringstream ss;
    ss << "\"" << name << "\": {\n";
    ss << clustering->ToJson() << "," << std::endl;
    ss << "\"objective function value\": " << objective_function_value << "," << std::endl;
    ss << "\"computation time seconds\": " << computation_time.count() << "}";
    return ss.str();
  }
};

std::string FormatComputationToJson(const IGraph &graph,
                                    const std::vector<ClusteringInfo> &computation_results,
                                    const unsigned size,
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

int main(int argc, char *argv[]) {
  if (argc != 5) {
    throw std::logic_error("expected 5 args, actual = " + std::to_string(argc));
  }
  unsigned graph_size = std::stoul(argv[1]);
  double density = std::stod(argv[2]);
  unsigned num_threads = std::stoul(argv[3]);
  unsigned num_graphs = std::stoul(argv[4]);

  ErdosRenyiRandomGraphFactory graphs_factory(density);
  std::shared_ptr<BinaryClusteringFactory> factory(new BinaryClusteringFactory);

  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::uniform_int_distribution<> dis_;

  const auto dir_name = "n-" + std::to_string(graph_size) + "-p-" + std::to_string(density) + "/";
  std::filesystem::create_directory(dir_name);

  for (unsigned i = 0; i < num_graphs; ++i) {
    auto graph = graphs_factory.CreateGraph(graph_size);
    std::vector<ClusteringInfo> infos;

    // NNLS algorithm
    ns2cc::NNLSAlgorithmForNS2CC nnls_algoritm(num_threads, factory);
    auto nnls_start_time = std::chrono::steady_clock::now();
    auto nnls_clustering = nnls_algoritm.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "NNLS",
        nnls_clustering,
        nnls_clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - nnls_start_time)
    );

    //N1LS
    ns2cc::N1LSAlgorithmForNS2CC n1ls_algorithm(num_threads, factory);
    auto n1ls_start_time = std::chrono::steady_clock::now();
    auto nn1s_clustering = n1ls_algorithm.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "N1LS",
        nn1s_clustering,
        nn1s_clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - n1ls_start_time)
    );

    ns2cc::NAlgorithmForNS2CC n_algorithm(num_threads, factory);
    auto n_start_time = std::chrono::steady_clock::now();
    auto n_clustering = n_algorithm.getBestNeighborhoodClustering(*graph);
    infos.emplace_back(
        "N",
        n_clustering,
        n_clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - n_start_time)
    );

    //BB algorithm
    ns2cc::BBAlgorithmForNS2CC bb_algorithm;
    auto bb_start_time = std::chrono::steady_clock::now();
    auto bbm_record = bb_algorithm.GetBestClustering(graph, nnls_clustering);
    infos.emplace_back(
        "BB",
        bbm_record,
        bbm_record->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - bb_start_time)
    );
    std::string result = FormatComputationToJson(*graph, infos, graph_size, density);

    std::ofstream out;
    std::stringstream name;
    name << dir_name << "n-" << graph_size << "-p-" << density << "-" << dis_(gen_) << ".json";
    out.open(name.str());
    out << result;
    out.close();
  }
}