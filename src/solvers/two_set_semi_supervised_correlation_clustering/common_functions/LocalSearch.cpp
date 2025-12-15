#include <algorithm>
#include <climits>

#include "../../../../include/solvers/two_set_semi_supervised_correlation_clustering/common_functions/LocalSearch.hpp"

IClustPtr set_semi_supervised_2cc::LocalSearch::ComputeLocalOptimum(const IGraph &graph,
                                                                    const IClustPtr &cur_clustering,
                                                                    const std::vector<unsigned> &first_cluster_vertices,
                                                                    const std::vector<unsigned> &second_cluster_vertices) {
  auto result = cur_clustering->GetCopy();
  while (true) {
    int local_improvement = INT_MIN;
    unsigned candidate = UINT_MAX;
    for (unsigned i = 0; i < graph.Size(); i++) {
      if (IsVertexInSet(i, first_cluster_vertices) || IsVertexInSet(i, second_cluster_vertices)) continue;
      if (const auto tmp_local_improvement = ComputeLocalImprovement(graph, result, i); tmp_local_improvement > local_improvement) {
        local_improvement = tmp_local_improvement;
        candidate = i;
      }
    }
    if (local_improvement <= 0) {
      break;
    }
    if (result->GetLabel(candidate) == FIRST_CLUSTER) {
      result->SetupLabelForVertex(candidate, SECOND_CLUSTER);
    } else {
      result->SetupLabelForVertex(candidate, FIRST_CLUSTER);
    }
  }
  return result;
}

int set_semi_supervised_2cc::LocalSearch::ComputeLocalImprovement(const IGraph &graph,
                                                                  const IClustPtr &cur_clustering,
                                                                  const unsigned vertex) {
  int local_improvement = 0;
  for (unsigned i = 0; i < graph.Size(); i++) {
    if (i == vertex) {
      continue;
    }
    const bool is_same_clustered = cur_clustering->IsSameClustered(i, vertex);
    const bool is_joined = graph.IsJoined(i, vertex);
    if (is_same_clustered) {
      local_improvement += is_joined ? -1 : 1;
    } else {
      local_improvement += is_joined ? 1 : -1;
    }
  }
  return local_improvement;
}

bool set_semi_supervised_2cc::LocalSearch::IsVertexInSet(const unsigned int vertex,
                                                         const std::vector<unsigned> &vertices_set) {
  return std::ranges::find(vertices_set, vertex) != vertices_set.end();
}
