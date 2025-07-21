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

std::vector<IGraphPtr> generate_graphs(ErdosRenyiRandomGraphFactory &graph_factory,
                                       const std::shared_ptr<TripleClusteringFactory>& clustering_factory,
                                       unsigned n,
                                       unsigned size) {
  std::vector<IGraphPtr> res;
  unsigned num_threads = 8;
  auto algo_n = non_strict_3cc::TwoVerticesNeighborhoodWithLocalSearch(num_threads, clustering_factory);
  auto algo_b = non_strict_3cc::BranchAndBounds();
  while (res.size() < n) {
    auto graph = graph_factory.CreateGraph(size);
    res.emplace_back(graph);
//    auto best_n= algo_n.getBestNeighborhoodClustering(*graph);
//    auto dist_n = best_n->GetDistanceToGraph(*graph);
//    auto best_b = algo_b.GetBestClustering(graph, best_n);
//    auto dist_b = best_b->GetDistanceToGraph(*graph);
//
//    if (dist_b < dist_n) {
//      res.emplace_back(graph);
//    }
  }
  return res;
}

int main() {
  unsigned num_threads = 8;
  double density = 0.25;
  ErdosRenyiRandomGraphFactory graphs_factory(density);

  std::shared_ptr<TripleClusteringFactory> factory(new TripleClusteringFactory);
  auto graphs = generate_graphs(graphs_factory, factory, 50, 20);
  std::cout << "Stop generate graphs" << std::endl;

  auto algo_n = non_strict_3cc::TwoVerticesNeighborhoodWithLocalSearch(num_threads, factory);
  auto algo_g = non_strict_3cc::GeneticAlgorithm(5000, 100, factory, 2048, 40, 1e-3);
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

//    unsigned num_clust_n = 0;
//    unsigned num_clust_b = 0;
//    unsigned num_clust_g = 0;
//
//    for (int i = 0; i < 3; i++) {
//        if (best_n->GetNumVerticesByLabel((ClusterLabels) i) > 0) {
//          num_clust_n++;
//        }
//        if (best_b->GetNumVerticesByLabel((ClusterLabels) i) > 0) {
//          num_clust_b++;
//        }
//        if (best_g.clustering->GetNumVerticesByLabel((ClusterLabels) i) > 0) {
//          num_clust_g++;
//        }
//    }
//    std::cout << num_clust_n << " " << num_clust_b << " " << num_clust_g << std::endl;


    std::cout << (double) dist_n / dist_b
              << " " << (double) dist_g / dist_b
              << " " << (double) dist_b / dist_b
              << " " << time
              << std::endl;
  }
}
