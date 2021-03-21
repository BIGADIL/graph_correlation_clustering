#include <climits>

#include "../../../../include/solvers/strict_two_correlation_clustering/common_functions/LocalSearch.hpp"

IClustPtr strict_2cc::LocalSearch::ComputeLocalOptimum(const IGraph &graph,
                                                       const IClustPtr &cur_clustering,
                                                       const unsigned vertex,
                                                       const unsigned opposite_vertex) {
  auto result = cur_clustering->GetCopy();
  auto exclude_vertices = ExcludeVertices(vertex, opposite_vertex);
  auto local_improvement_list = InitLocalImprovements(graph, cur_clustering, exclude_vertices);
  while (true) {
    auto candidate = FindCandidate(graph, local_improvement_list, exclude_vertices);
    if (candidate.local_improvement <= 0) {
      break;
    }
    local_improvement_list = UpdateLocalImprovements(graph, result, local_improvement_list, candidate.vertex, exclude_vertices);
    result = UpdateClustering(result, candidate.vertex);
  }
  return result;
}

std::vector<int> strict_2cc::LocalSearch::InitLocalImprovements(const IGraph &graph,
                                                                const IClustPtr &cur_clustering,
                                                                const strict_2cc::LocalSearch::ExcludeVertices exclude_vertices) {
  std::vector<int> local_improvement_list(graph.Size());
  for (unsigned i = 0; i < graph.Size(); i++) {
    if (exclude_vertices.contain(i)) {
      continue;
    }
    for (unsigned j = i + 1; j < graph.Size(); ++j) {
      bool is_same_clustered = cur_clustering->IsSameClustered(i, j);
      bool is_joined = graph.IsJoined(i, j);
      if (is_same_clustered) {
        local_improvement_list[i] += is_joined ? -1 : 1;
        local_improvement_list[j] += is_joined ? -1 : 1;
      } else {
        local_improvement_list[i] += is_joined ? 1 : -1;
        local_improvement_list[j] += is_joined ? 1 : -1;
      }
    }
  }
  return local_improvement_list;
}

strict_2cc::LocalSearch::LocalSearchCandidate strict_2cc::LocalSearch::FindCandidate(const IGraph &graph,
                                                                                     const std::vector<int> &local_improvement_list,
                                                                                     const strict_2cc::LocalSearch::ExcludeVertices exclude_vertices) {
  int local_improvement = INT_MIN;
  unsigned candidate = UINT_MAX;
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if (exclude_vertices.contain(i)) {
      continue;
    }
    if (local_improvement_list[i] > local_improvement) {
      local_improvement = local_improvement_list[i];
      candidate = i;
    }
  }
  return LocalSearchCandidate(candidate, local_improvement);
}

std::vector<int> strict_2cc::LocalSearch::UpdateLocalImprovements(const IGraph &graph,
                                                                  const IClustPtr &cur_clustering,
                                                                  std::vector<int> &local_improvement_list,
                                                                  const unsigned int vertex,
                                                                  const strict_2cc::LocalSearch::ExcludeVertices exclude_vertices) {
  local_improvement_list[vertex] = 0;
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if (i == vertex || exclude_vertices.contain(i)) {
      continue;
    }
    if ((graph.IsJoined(i, vertex) && cur_clustering->IsSameClustered(i, vertex))
        || (!graph.IsJoined(i, vertex) && !cur_clustering->IsSameClustered(i, vertex))) {
      local_improvement_list[i] += 2;
      local_improvement_list[vertex] += 1;
    } else {
      local_improvement_list[i] -= 2;
      local_improvement_list[vertex] -= 1;
    }
  }
  return local_improvement_list;
}

IClustPtr strict_2cc::LocalSearch::UpdateClustering(IClustPtr &cur_clustering, const unsigned int vertex) {
  if (cur_clustering->GetLabel(vertex) == FIRST_CLUSTER) {
    cur_clustering->SetupLabelForVertex(vertex, SECOND_CLUSTER);
  } else {
    cur_clustering->SetupLabelForVertex(vertex, FIRST_CLUSTER);
  }
  return cur_clustering;
}
