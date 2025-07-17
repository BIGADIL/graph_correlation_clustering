#include <iostream>
#include <thread>
#include <climits>
#include <random>
#include <algorithm>
#include <set>

#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/clustering/factories/TripleClusteringFactory.hpp"
#include "include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithLocalSearch.hpp"
#include "include/solvers/non_strict_three_correlation_clustering/common_functions/LocalSearch.hpp"
#include "include/solvers/non_strict_three_correlation_clustering/genetic_algorithms/GeneticAlgorithm.hpp"
#include "include/solvers/non_strict_three_correlation_clustering/clust_algorithms/BranchAndBounds.hpp"
#include "include/solvers/non_strict_three_correlation_clustering/clust_algorithms/BrutForce.hpp"

std::vector<IGraphPtr> generate_graphs(ErdosRenyiRandomGraphFactory &factory, unsigned n, unsigned size) {
  std::vector<IGraphPtr> res;
  while (res.size() < n) {
    auto graph = factory.CreateGraph(size);
    res.emplace_back(graph);
  }
  return res;
}

int main() {
  unsigned num_threads = 8;
  double density = 0.25;
  ErdosRenyiRandomGraphFactory graphs_factory(density);

  std::shared_ptr<TripleClusteringFactory> factory(new TripleClusteringFactory);
  auto graphs = generate_graphs(graphs_factory, 50, 30);

  auto algo_n = non_strict_3cc::TwoVerticesNeighborhoodWithLocalSearch(num_threads, factory);
  auto algo_g = non_strict_3cc::GeneticAlgorithm(1000, 100, factory, 1024, 20, 1e-3);
  auto algo_b = non_strict_3cc::BranchAndBounds();

  for (auto &graph: graphs) {
    auto best_n= algo_n.getBestNeighborhoodClustering(*graph);
    auto dist_n = best_n->GetDistanceToGraph(*graph);

    auto start_time = std::chrono::steady_clock::now();
    auto best_g = algo_g.Train(graph);
    auto time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time).count();
    auto dist_g = best_g.clustering->GetDistanceToGraph(*graph);

    auto best_b = algo_b.GetBestClustering(graph, best_n);
    auto dist_b = best_b->GetDistanceToGraph(*graph);

//    if (((double) dist_n / dist_g) < 1) {
//      std::cout << " ";
//    }

    std::cout << (double) dist_n / dist_b
              << " " << (double) dist_g / dist_b
              << " " << (double) dist_b / dist_b
              << " " << time
              << std::endl;
  }
}
