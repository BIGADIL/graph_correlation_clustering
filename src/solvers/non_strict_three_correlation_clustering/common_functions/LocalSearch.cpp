#include <climits>
#include <stdexcept>
#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/LocalSearch.hpp"

std::vector<int> non_strict_3cc::LocalSearch::InitLocalImprovements(const IGraph &graph,
                                                                    const IClustPtr &cur_clustering,
                                                                    ClusterLabels first_label,
                                                                    ClusterLabels second_label) {
  std::vector<int> local_improvement_list(graph.Size());
  for (unsigned i = 0; i < graph.Size(); i++) {
    auto label = cur_clustering->GetLabel(i);
    if (label != first_label && label != second_label) continue;
    for (unsigned j = i + 1; j < graph.Size(); ++j) {
      auto labelJ = cur_clustering->GetLabel(j);
      if (labelJ != first_label && labelJ != second_label) continue;
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

non_strict_3cc::LocalSearch::LocalSearchCandidate non_strict_3cc::LocalSearch::FindCandidate(const IGraph &graph,
                                                                                             const std::vector<int> &local_improvement_list,
                                                                                             const IClustPtr &cur_clustering,
                                                                                             ClusterLabels first_label,
                                                                                             ClusterLabels second_label) {
  int local_improvement = INT_MIN;
  unsigned candidate = UINT_MAX;
  for (unsigned i = 0; i < graph.Size(); ++i) {
    auto label = cur_clustering->GetLabel(i);
    if (label != first_label && label != second_label) continue;
    if (local_improvement_list[i] > local_improvement) {
      local_improvement = local_improvement_list[i];
      candidate = i;
    }
  }
  return {candidate, local_improvement};
}

std::vector<int> non_strict_3cc::LocalSearch::UpdateLocalImprovements(const IGraph &graph,
                                                                      const IClustPtr &cur_clustering,
                                                                      std::vector<int> &local_improvement_list,
                                                                      const unsigned int vertex,
                                                                      ClusterLabels first_label,
                                                                      ClusterLabels second_label) {
  local_improvement_list[vertex] = 0;
  for (unsigned i = 0; i < graph.Size(); ++i) {
    auto label = cur_clustering->GetLabel(i);
    if (i == vertex || (label != first_label && label != second_label)) {
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

std::vector<int> non_strict_3cc::LocalSearch::ComputeLocalImprovement(const IGraph &graph,
                                                                      const IClustPtr &cur_clustering,
                                                                      const unsigned vertex) {
  auto vertex_label = cur_clustering->GetLabel(vertex);
  std::vector<int> local_improvements(3, 0);
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if (i == vertex) continue;
    bool is_same_clustered = cur_clustering->IsSameClustered(i, vertex);
    bool is_joined = graph.IsJoined(i, vertex);
    if (is_same_clustered) {
      for (unsigned j = 0; j < 3; ++j) {
        if (IsSkipLabelForVertex(vertex_label, j)) continue;
        local_improvements[j] += is_joined ? -1 : 1;
      }
    } else {
      auto idx = GetIdxByLabel(cur_clustering->GetLabel(i));
      local_improvements[idx] += is_joined ? 1 : -1;
    }
  }
  return local_improvements;
}

IClustPtr non_strict_3cc::LocalSearch::ComputeLocalOptimum(const IGraph &graph,
                                                           const IClustPtr &cur_clustering,
                                                           const ClusterLabels first_label,
                                                           const ClusterLabels second_label) {
  auto result = cur_clustering->GetCopy();
  auto local_improvement_list = InitLocalImprovements(graph, cur_clustering, first_label, second_label);
  while (true) {
    auto candidate = FindCandidate(graph, local_improvement_list, result, first_label, second_label);
    if (candidate.local_improvement <= 0) {
      break;
    }
    local_improvement_list =
        UpdateLocalImprovements(graph, result, local_improvement_list, candidate.vertex, first_label, second_label);
    if (result->GetLabel(candidate.vertex) == first_label) {
      result->SetupLabelForVertex(candidate.vertex, second_label);
    } else {
      result->SetupLabelForVertex(candidate.vertex, first_label);
    }
  }
  return result;
}

IClustPtr non_strict_3cc::LocalSearch::ComputeLocalOptimum(const IGraph &graph,
                                                           const IClustPtr &cur_clustering) {
  auto result = cur_clustering->GetCopy();
  while (true) {
    int local_improvement = INT_MIN;
    unsigned candidate = UINT_MAX;
    ClusterLabels label = NON_CLUSTERED;
    for (unsigned i = 0; i < graph.Size(); ++i) {
      auto tmp_local_improvements = ComputeLocalImprovement(graph, result, i);
      for (unsigned j = 0; j < 3; ++j) {
        if (result->GetLabel(i) == GetLabelByIdx(j)) continue;
        if (tmp_local_improvements[j] > local_improvement) {
          local_improvement = tmp_local_improvements[j];
          candidate = i;
          label = GetLabelByIdx(j);
        }
      }
    }
    if (local_improvement <= 0) break;
    result->SetupLabelForVertex(candidate, label);
  }
  return result;
}

bool non_strict_3cc::LocalSearch::IsSkipLabelForVertex(const ClusterLabels label,
                                                       const unsigned idx) {
  return (label == FIRST_CLUSTER && idx == 0)
      || (label == SECOND_CLUSTER && idx == 1)
      || (label == THIRD_CLUSTER && idx == 2);
}

unsigned non_strict_3cc::LocalSearch::GetIdxByLabel(const ClusterLabels labels) {
  switch (labels) {
    case FIRST_CLUSTER:return 0;
    case SECOND_CLUSTER:return 1;
    case THIRD_CLUSTER:return 2;
    default:throw std::logic_error("unsupported label");
  }
}
ClusterLabels non_strict_3cc::LocalSearch::GetLabelByIdx(const unsigned idx) {
  switch (idx) {
    case 0:return FIRST_CLUSTER;
    case 1:return SECOND_CLUSTER;
    case 2:return THIRD_CLUSTER;
    default:throw std::logic_error("unsupported idx");
  }
}
