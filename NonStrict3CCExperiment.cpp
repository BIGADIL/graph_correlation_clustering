#include <filesystem>
#include <fstream>
#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/clustering/factories/TernaryClusteringFactory.hpp"
#include "include/solvers/non_strict_three_correlation_clustering/NonStrict3CcSolver.hpp"
int main(int argc, char *argv[]) {
  if (argc != 5) {
    throw std::logic_error("expected 5 args, actual = " + std::to_string(argc));
  }
  unsigned graph_size = std::stoul(argv[1]);
  double density = std::stod(argv[2]);
  unsigned num_threads = std::stoul(argv[3]);
  unsigned num_graphs = std::stoul(argv[4]);

  ErdosRenyiRandomGraphFactory graphs_factory(density);
  std::shared_ptr<TernaryClusteringFactory> factory(new TernaryClusteringFactory);

  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::uniform_int_distribution<> dis_;

  std::filesystem::create_directory("non_strict_3cc");
  auto path = std::filesystem::current_path();
  path.append("non_strict_3cc");
  std::filesystem::current_path(path);
  const auto dir_name = "n-" + std::to_string(graph_size) + "-p-" + std::to_string(density) + "/";
  std::filesystem::create_directory(dir_name);

  non_strict_3cc::NonStrict3CCSolver solver(num_threads, factory);
  for (unsigned i = 0; i < num_graphs; ++i) {
    auto graph = graphs_factory.CreateGraph(graph_size);
    auto report = solver.solve(
        graph,
        density,
        {
            "NeighborhoodWithLocalSearch",
            "TwoVerticesNeighborhood",
            "TwoVerticesNeighborhoodWithLocalSearch",
            "BranchAndBounds",
            }
    );

    std::ofstream out;
    std::stringstream name;
    name << dir_name << "n-" << graph_size << "-p-" << density << "-" << dis_(gen_) << ".json";
    out.open(name.str());
    out << report;
    out.close();
  }
}