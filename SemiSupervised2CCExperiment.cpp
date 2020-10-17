#include <iostream>
#include <filesystem>

#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/clustering/factories/IClusteringFactory.hpp"
#include "include/clustering/factories/BinaryClusteringFactory.hpp"
#include "include/solvers/two_semi_supervised_correlation_clustering/SemiSupervised2CCSolver.hpp"
#include "include/common/ExperimentParameters.hpp"

std::pair<unsigned, unsigned> getTwoRandomVertices(const IGraph &graph) {
  auto size = graph.Size();
  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::uniform_int_distribution<> dis_(0, size - 1);
  unsigned x, y;
  do {
    x = dis_(gen_);
    y = dis_(gen_);
  } while (x == y);
  return std::make_pair(x, y);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    throw std::logic_error("expected 2 args, actual = " + std::to_string(argc));
  }
  ExperimentParameters ep = ExperimentParameters::readFromConfig(argv[1]);
  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::uniform_int_distribution<> dis_;

  std::filesystem::create_directory("semi_supervised_2cc");
  auto path = std::filesystem::current_path();
  path.append("semi_supervised_2cc");
  std::filesystem::current_path(path);

  std::shared_ptr<BinaryClusteringFactory> factory(new BinaryClusteringFactory);
  for (const auto &graph_size: ep.GetGraphSize()) {
    for (const auto &density: ep.GetDensity()) {
      ErdosRenyiRandomGraphFactory graphs_factory(density);
      const auto dir_name = "n-" + std::to_string(graph_size) + "-p-" + std::to_string(density) + "/";
      std::filesystem::create_directory(dir_name);
      semi_supervised_2cc::SemiSupervised2CCSolver solver(ep.GetNumThreads(), factory);
      for (unsigned i = 0; i < ep.GetNumGraphs(); ++i) {
        auto graph = graphs_factory.CreateGraph(graph_size);
        auto pre_clustered_vertices = getTwoRandomVertices(*graph);
        auto report = solver.solve(
            graph,
            density,
            ep.GetAlgorithms(),
            pre_clustered_vertices.first,
            pre_clustered_vertices.second
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