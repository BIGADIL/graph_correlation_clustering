#include <iostream>
#include <random>
#include <set>

#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/graphs/factories/StackOverflowGraphFactory.hpp"
#include "include/solvers/non_strict_two_correlation_clustering/ipls_algorithms/IPLSAlgorithm.hpp"
#include "include/clustering/factories/BinaryClusteringFactory.hpp"
#include "include/solvers/non_strict_three_correlation_clustering/ipls_algorithms/IPLSAlgorithm.hpp"

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
      graphs_factory(R"(D:\run\graph_correlation_clustering\non_strict_2cc\Tags_so.json)", "jaccard_threshold");

  std::shared_ptr<TripleClusteringFactory> factory(new TripleClusteringFactory);
  auto graphs = generate_graphs(graphs_factory, 100, 100);

  auto early_stops = {6};
  auto population_sizes = {128, 256, 512};
  auto tournament_sizes = {5, 10, 15};
  auto mutations = {1e-2, 1e-1, 2e-1, 3e-1, 4e-1};

  for (auto &early_stop: early_stops) {
    for (auto &population_size: population_sizes) {
      for (auto &tournament_size: tournament_sizes) {
        for (auto &mutation: mutations) {
          auto algo_g = non_strict_3cc::IPLSAlgorithm(5000,
                                                      early_stop,
                                                      factory,
                                                      population_size,
                                                      tournament_size,
                                                      mutation,
                                                      num_threads);
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
