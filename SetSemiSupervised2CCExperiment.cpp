#include <random>
#include <filesystem>
#include <algorithm>

#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/clustering/factories/IClusteringFactory.hpp"
#include "include/clustering/factories/BinaryClusteringFactory.hpp"
#include "include/solvers/two_set_semi_supervised_correlation_clustering/SetSemiSupervised2CCSolver.hpp"
#include "include/common/ExperimentParameters.hpp"

std::pair<std::vector<unsigned>, std::vector<unsigned >> GetPreClusteredSets(const IGraph &graph,
                                                                             const std::vector<double> &parts) {
  if (parts.size() != 2) {
    throw std::logic_error("expect size = 2");
  }
  if (parts[0] + parts[1] > 1) {
    throw std::logic_error("to big parts");
  }
  auto size = graph.Size();
  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::uniform_int_distribution<> dis_(0, size - 1);

  unsigned first_cluster_part = std::max(unsigned(parts[0] * size), 1U);
  unsigned second_cluster_part = std::max(unsigned(parts[1] * size), 1U);

  std::vector<unsigned> all_peeked;
  std::vector<unsigned> first_pre_cluster;
  std::vector<unsigned> second_pre_cluster;

  while (first_pre_cluster.size() != first_cluster_part) {
    auto x = dis_(gen_);
    if (std::find(all_peeked.begin(), all_peeked.end(), x) == all_peeked.end()) {
      all_peeked.push_back(x);
      first_pre_cluster.push_back(x);
    }
  }

  while (second_pre_cluster.size() != second_cluster_part) {
    auto x = dis_(gen_);
    if (std::find(all_peeked.begin(), all_peeked.end(), x) == all_peeked.end()) {
      all_peeked.push_back(x);
      second_pre_cluster.push_back(x);
    }
  }
  return std::make_pair(first_pre_cluster, second_pre_cluster);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    throw std::logic_error("expected 2 args, actual = " + std::to_string(argc));
  }
  ExperimentParameters ep = ExperimentParameters::readFromConfig(argv[1]);
  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::uniform_int_distribution<> dis_;

  std::filesystem::create_directory("set_semi_supervised_2cc");
  auto path = std::filesystem::current_path();
  path.append("set_semi_supervised_2cc");
  std::filesystem::current_path(path);

  std::shared_ptr<BinaryClusteringFactory> factory(new BinaryClusteringFactory);
  for (const auto &graph_size: ep.GetGraphSize()) {
    for (const auto &density: ep.GetDensity()) {
      ErdosRenyiRandomGraphFactory graphs_factory(density);
      const auto dir_name = "n-" + std::to_string(graph_size) + "-p-" + std::to_string(density) + "/";
      std::filesystem::create_directory(dir_name);
      set_semi_supervised_2cc::SetSemiSupervised2CCSolver solver(ep.GetNumThreads(), factory);
      for (unsigned i = 0; i < ep.GetNumGraphs(); ++i) {
        auto graph = graphs_factory.CreateGraph(graph_size);
        auto pre_clusters = GetPreClusteredSets(*graph, ep.GetParts());
        auto report = solver.solve(
            graph,
            density,
            ep.GetAlgorithms(),
            pre_clusters.first,
            pre_clusters.second
        );
        std::ofstream out;
        std::stringstream name;
        name << dir_name
             << "n-" << graph_size
             << "-p-" << density
             << "-c1-" << ep.GetParts()[0]
             << "-c2-" << ep.GetParts()[1]
             << "-" << dis_(gen_) << ".json";
        out.open(name.str());
        out << report;
        out.close();
      }
    }
  }
}