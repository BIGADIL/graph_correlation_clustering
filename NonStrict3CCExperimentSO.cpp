#include <filesystem>
#include <fstream>

#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/clustering/factories/TripleClusteringFactory.hpp"
#include "include/solvers/non_strict_three_correlation_clustering/NonStrict3CcSolver.hpp"
#include "include/common/ExperimentParameters.hpp"
#include "include/graphs/factories/StackOverflowGraphFactory.hpp"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    throw std::logic_error("expected 3 args, actual = " + std::to_string(argc));
  }
  ExperimentParameters ep = ExperimentParameters::readFromConfig(argv[1]);
  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::uniform_int_distribution<> dis_;

  std::string data_set = argv[2];

  std::filesystem::create_directory("non_strict_3cc");
  auto path = std::filesystem::current_path();
  path.append("non_strict_3cc");
  std::filesystem::current_path(path);

  std::shared_ptr<TripleClusteringFactory> factory(new TripleClusteringFactory);
  for (const auto &graph_size: ep.GetGraphSize()) {
    for (const auto &distribution: ep.GetDistribution()) {
      StackOverflowGraphFactory graphs_factory(std::filesystem::current_path().string() + "/" + data_set, distribution);
      const auto dir_name = "n-" + std::to_string(graph_size) + "-ds-" + data_set + "-dist-" + distribution + "/";
      std::filesystem::create_directory(dir_name);
      non_strict_3cc::NonStrict3CCSolver solver(ep.GetNumThreads(), factory);
      for (unsigned i = 0; i < ep.GetNumGraphs(); ++i) {
        auto graph = graphs_factory.CreateGraph(graph_size);
        auto report = solver.solve(
            graph,
            distribution,
            ep.GetAlgorithms()
        );
        std::ofstream out;
        std::stringstream name;
        name << dir_name << "n-" << graph_size << "-ds-" << data_set << "-dist-" << distribution << "-" << dis_(gen_) << ".json";
        out.open(name.str());
        out << report;
        out.close();
      }
    }
  }
}