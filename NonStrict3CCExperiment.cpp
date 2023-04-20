#include <filesystem>
#include <fstream>

#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/clustering/factories/TripleClusteringFactory.hpp"
#include "include/solvers/non_strict_three_correlation_clustering/NonStrict3CcSolver.hpp"
#include "include/common/ExperimentParameters.hpp"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    throw std::logic_error("expected 2 args, actual = " + std::to_string(argc));
  }
  ExperimentParameters ep = ExperimentParameters::readFromConfig(argv[1]);
  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::uniform_int_distribution<> dis_;

  std::filesystem::create_directory("non_strict_3cc");
  auto path = std::filesystem::current_path();
  path.append("non_strict_3cc");
  std::filesystem::current_path(path);

  std::shared_ptr<TripleClusteringFactory> factory(new TripleClusteringFactory);
  for (const auto &graph_size: ep.GetGraphSize()) {
    for (const auto &density: ep.GetDensity()) {
      ErdosRenyiRandomGraphFactory graphs_factory(density);
      const auto dir_name = "n-" + std::to_string(graph_size) + "-p-" + std::to_string(density) + "/";
      std::filesystem::create_directory(dir_name);
      non_strict_3cc::NonStrict3CCSolver solver(ep.GetNumThreads(), factory);
      for (unsigned i = 0; i < ep.GetNumGraphs(); ++i) {
        auto graph = graphs_factory.CreateGraph(graph_size);
        auto report = solver.solve(
            graph,
            density,
            ep.GetAlgorithms()
        );
        std::ofstream out;
        std::stringstream name;
        name << dir_name << "n-" << graph_size << "-p-" << density << "-" << dis_(gen_) << ".json";
        out.open(name.str());
        out << report;
        out.close();
      }
    }
  }
}