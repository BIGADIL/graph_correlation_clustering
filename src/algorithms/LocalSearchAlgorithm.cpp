#include <climits>
#include <iostream>

#include "../../include/algorithms/LocalSearchAlgorithm.hpp"

LocalSearchAlgorithm::LocalSearchAlgorithm(std::shared_ptr<IClusteringFactory> clustering_factory)
    : clustering_factory_(std::move(clustering_factory)) {

}
IClusteringPointer LocalSearchAlgorithm::ComputeLocalOptimum(const IGraph &graph,
                                                             const IClusteringPointer &cur_clustering) const {
  auto result = cur_clustering->GetCopy();
  while (true) {
    int local_improvement = INT_MIN;
    int candidate = -1;
    for (unsigned i = 0; i < graph.Size(); i++) {
      auto tmp_local_improvement = ComputeLocalImprovement(graph, result, i);
      if (tmp_local_improvement > local_improvement) {
        local_improvement = tmp_local_improvement;
        candidate = i;
      }
    }
    if (local_improvement <= 0) {
      break;
    }
    if (result->GetLabel(candidate) == 0) {
      result->SetupLabelForVertex(candidate, SECOND_CLUSTER);
    } else {
      result->SetupLabelForVertex(candidate, FIRST_CLUSTER);
    }
  }

  return result;
}
int LocalSearchAlgorithm::ComputeLocalImprovement(const IGraph &graph,
                                                  const IClusteringPointer &cur_clustering,
                                                  const unsigned vertex) const {
  int local_improvement = 0;
  for (unsigned i = 0; i < graph.Size(); i++) {
    if (i == vertex) {
      continue;
    }
    bool is_same_clustered = cur_clustering->IsSameClustered(i, vertex);
    bool is_joined = graph.IsJoined(i, vertex);
    if (is_same_clustered) {
      if (is_joined) {
        local_improvement--;
      } else {
        local_improvement++;
      }
    } else {
      if (is_joined) {
        local_improvement++;
      } else {
        local_improvement--;
      }
    }
  }
  return local_improvement;
}
