#include <climits>
#include "../../../include/algorithms/strict_two_correlation_clustering/LSAlgorithmForS2CC.hpp"

IClustPtr LSAlgorithmForS2CC::ComputeLocalOptimum(const IGraph &graph,
                                                   const IClustPtr &cur_clustering,
                                                   const unsigned vertex,
                                                   const unsigned opposite_vertex) {
  auto result = cur_clustering->GetCopy();
  while (true) {
    int local_improvement = INT_MIN;
    unsigned candidate = UINT_MAX;
    for (unsigned i = 0; i < graph.Size(); i++) {
      if (i == vertex || i == opposite_vertex) continue;
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

int LSAlgorithmForS2CC::ComputeLocalImprovement(const IGraph &graph,
                                                 const IClustPtr &cur_clustering,
                                                 const unsigned vertex) {
  int local_improvement = 0;
  for (unsigned i = 0; i < graph.Size(); i++) {
    if (i == vertex) {
      continue;
    }
    bool is_same_clustered = cur_clustering->IsSameClustered(i, vertex);
    bool is_joined = graph.IsJoined(i, vertex);
    if (is_same_clustered) {
      local_improvement += is_joined ? -1 : 1;
    } else {
      local_improvement += is_joined ? 1 : -1;
    }
  }
  return local_improvement;
}
