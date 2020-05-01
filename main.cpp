#include <iostream>
#include <chrono>
#include <thread>
#include "include/algorithms/NeighborhoodAlgorithm.hpp"
#include "include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"
#include "include/clustering/factories/BinaryClusteringFactory.hpp"
#include "include/algorithms/BranchAndBoundAlgorithm.hpp"

int main() {
  ErdosRenyiRandomGraphFactory graphs_factory(0.33);
  std::shared_ptr<BinaryClusteringFactory> factory(new BinaryClusteringFactory);

  auto graph = graphs_factory.CreateGraph(FIXED_VECTOR_SIZE);

  NeighborhoodAlgorithm neighborhood_algorithm(8, factory);

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  auto best_clustering = neighborhood_algorithm.getBestNeighborhoodClustering(*graph);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

  std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
  auto record = best_clustering->GetDistanceToGraph(*graph);
  std::cout << "distance=" << record << std::endl;

  BranchAndBoundAlgorithm branch_and_bound_algorithm;
  begin = std::chrono::steady_clock::now();
  auto bbm_record = branch_and_bound_algorithm.GetBestClustering(graph, record);
  end = std::chrono::steady_clock::now();
  std::cout << "GetBestClustering time difference = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
  std::cout << "bbm record=" << bbm_record << std::endl;
}