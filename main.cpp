#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>

#include "include/algorithms/non_strict_two_correlation_clustering/NWithLSAlgorithm.hpp"
#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/clustering/factories/BinaryClusteringFactory.hpp"
#include "include/algorithms/non_strict_two_correlation_clustering/BBAlgorithm.hpp"

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
    NWithLSAlgorithm neighborhood_algorithm(num_threads, factory);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto best_clustering = neighborhood_algorithm.getBestNeighborhoodClustering(*graph);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    ClusteringInfo approximate_clustering_info(
        "2-clustering-neighborhood-plus-local-search",
        best_clustering,
        best_clustering->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(end - begin)
    );

    BBAlgorithm bb_algorithm;
    begin = std::chrono::steady_clock::now();
    auto bbm_record = bb_algorithm.GetBestClustering(graph, best_clustering);
    end = std::chrono::steady_clock::now();

    ClusteringInfo branch_bounds_clustering_info(
        "2-clustering-brands-and-bounds",
        bbm_record,
        bbm_record->GetDistanceToGraph(*graph),
        std::chrono::duration_cast<std::chrono::seconds>(end - begin)
    );
    std::vector<ClusteringInfo> infos{approximate_clustering_info, branch_bounds_clustering_info};
    std::string result = FormatComputationToJson(*graph, infos, graph_size, density);

    std::ofstream out;
    std::stringstream name;
    name << dir_name << "n-" << graph_size <<"-p-" << density << "-" << dis_(gen_) << ".json";
    out.open(name.str());
    out << result;
    out.close();
  }
}