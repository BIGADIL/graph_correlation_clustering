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
#include "include/graphs/factories/StackOverflowGraphFactory.hpp"

std::vector<IGraphPtr> generate_graphs(StackOverflowGraphFactory &graph_factory,
                                       unsigned n,
                                       unsigned size) {
  std::vector<IGraphPtr> res;
  while (res.size() < n) {
    auto graph = graph_factory.CreateGraph(size);
    res.emplace_back(graph);
  }
  return res;
}

int main() {
  unsigned num_threads = 8;
  StackOverflowGraphFactory
      graphs_factory(R"(D:\run\graph_correlation_clustering\non_strict_3cc\Tags_10.json)", "threshold");

  std::shared_ptr<TripleClusteringFactory> factory(new TripleClusteringFactory);
  auto graphs = generate_graphs(graphs_factory, 30, 100);

  auto early_stops = {10};
  auto population_sizes = {512, 1024, 2048, 4096};
  auto tournament_sizes = {5, 10, 20, 40};
  auto mutations = {1e-1};

  for (auto &early_stop: early_stops) {
    for (auto &population_size: population_sizes) {
      for (auto &tournament_size: tournament_sizes) {
        for (auto &mutation: mutations) {
          auto algo_g = non_strict_3cc::GeneticAlgorithm(5000,
                                                         early_stop,
                                                         factory,
                                                         population_size,
                                                         tournament_size,
                                                         mutation);
          std::cout << 1 << std::endl;
          double sum = 0;
          for (auto &graph: graphs) {
            auto best_g = algo_g.Train(graph);
            sum += best_g.clustering->GetDistanceToGraph(*graph);
          }
          sum /= graphs.size();
          std::cout << "early_stop=" << early_stop
                    << ", population_size=" << population_size
                    << ", tournament_size=" << tournament_size
                    << ", mutation=" << mutation
                    << ", average_obj=" << sum << std::endl;

        }
      }
    }
  }
}
