#include <climits>
#include <utility>
#include <vector>

#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/BrutForce.hpp"

IClustPtr non_strict_3cc::BrutForce::GetBestClustering(const IGraphPtr &graph) {
  int base = 3;
  std::vector<int> clustering(graph->Size(), 0);
  std::vector<int> one(graph->Size(), 0);
  one[0] = 1;
  std::vector<int> best_clustering;
  unsigned best_distance = UINT_MAX;
  while (clustering.back() != 1) {
    auto tmp_distance = GetDistanceToGraph(*graph, clustering);
    if (tmp_distance < best_distance) {
      best_distance = tmp_distance;
      best_clustering = clustering;
    }
    for (unsigned i = 0; i < clustering.size(); ++i) {
      clustering[i] += one[i];
      if (i != clustering.size() - 1) {
        clustering[i + 1] += clustering[i] / base;
      }
      clustering[i] %= base;
    }
  }
  auto result = factory_->CreateClustering(graph->Size());
  for (unsigned i = 0; i < graph->Size(); ++i) {
    switch (best_clustering[i]) {
      case 0:result->SetupLabelForVertex(i, FIRST_CLUSTER);
        break;
      case 1:result->SetupLabelForVertex(i, SECOND_CLUSTER);
        break;
      case 2:result->SetupLabelForVertex(i, THIRD_CLUSTER);
        break;
      default:throw std::logic_error("Unsupported");
    }
  }
  return result;
}

non_strict_3cc::BrutForce::BrutForce(IClustFactoryPtr factory) :
    factory_(std::move(factory)) {

}

unsigned non_strict_3cc::BrutForce::GetDistanceToGraph(IGraph &graph, const std::vector<int> &clustering) {
  unsigned distance = 0;
  for (unsigned i = 0; i < graph.Size(); i++) {
    for (unsigned j = i + 1; j < graph.Size(); j++) {
      if ((clustering[i] != clustering[j] && graph.IsJoined(i, j))
          || (clustering[i] == clustering[j] && !graph.IsJoined(i, j))) {
        distance++;
      }
    }
  }
  return distance;
}
