#include <climits>
#include <iostream>
#include <vector>

#include "../../../../include/solvers/non_strict_two_correlation_clustering/common_functions/LocalSearch.hpp"

IClustPtr non_strict_2cc::LocalSearch::ComputeLocalOptimum(const IGraph &graph,
                                                           const IClustPtr &cur_clustering) {
  auto result = cur_clustering->GetCopy();
  auto local_improvement_list = InitLocalImprovements(graph, cur_clustering);
  while (true) {
    auto candidate = FindCandidate(graph, local_improvement_list);
    if (candidate.local_improvement <= 0) {
      break;
    }
    local_improvement_list = UpdateLocalImprovements(graph, result, local_improvement_list, candidate.vertex);
    result = UpdateClustering(result, candidate.vertex);
  }
  return result;
}

std::vector<int> non_strict_2cc::LocalSearch::InitLocalImprovements(const IGraph &graph,
                                                                    const IClustPtr &cur_clustering) {
  std::vector<int> local_improvement_list(graph.Size());
  for (unsigned i = 0; i < graph.Size(); i++) {
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

non_strict_2cc::LocalSearch::LocalSearchCandidate non_strict_2cc::LocalSearch::FindCandidate(const IGraph &graph,
                                                                                             const std::vector<int> &local_improvement_list) {
  int local_improvement = INT_MIN;
  unsigned candidate = UINT_MAX;
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if (local_improvement_list[i] > local_improvement) {
      local_improvement = local_improvement_list[i];
      candidate = i;
    }
  }
  return LocalSearchCandidate(candidate, local_improvement);
}

std::vector<int> non_strict_2cc::LocalSearch::UpdateLocalImprovements(const IGraph &graph,
                                                                      const IClustPtr &cur_clustering,
                                                                      std::vector<int> &local_improvement_list,
                                                                      const unsigned vertex) {
  local_improvement_list[vertex] = 0;
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if (i == vertex) {
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

IClustPtr non_strict_2cc::LocalSearch::UpdateClustering(IClustPtr &cur_clustering,
                                                        const unsigned int vertex) {
  if (cur_clustering->GetLabel(vertex) == FIRST_CLUSTER) {
    cur_clustering->SetupLabelForVertex(vertex, SECOND_CLUSTER);
  } else {
    cur_clustering->SetupLabelForVertex(vertex, FIRST_CLUSTER);
  }
  return cur_clustering;
}
