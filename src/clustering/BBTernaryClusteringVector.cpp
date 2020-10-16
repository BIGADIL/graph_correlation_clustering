#include <climits>
#include "../../include/clustering/BBTernaryClusteringVector.hpp"

BBTernaryClusteringVector::BBTernaryClusteringVector(const unsigned size,
                                                     const std::shared_ptr<IGraph> &graph)
    : TernaryClusteringVector(size), graph_(graph) {
  obj_func_value_increase_relatively_to_first_cluster_ = std::vector<int>(size);
  obj_func_value_increase_relatively_to_second_cluster_ = std::vector<int>(size);
  obj_func_value_increase_relatively_to_third_cluster_ = std::vector<int>(size);
  number_of_neighbours_in_non_clustered_graph_ = std::vector<int>(size);
  obj_func_value_on_partially_built_clustering_ = 0;
  for (unsigned i = 0; i < graph->Size(); i++) {
    for (unsigned j = i + 1; j < graph->Size(); j++) {
      if (graph->IsJoined(i, j)) {
        num_of_edges_of_non_constructed_graph_++;
        number_of_neighbours_in_non_clustered_graph_[i]++;
        number_of_neighbours_in_non_clustered_graph_[j]++;
      }
    }
  }
}

void BBTernaryClusteringVector::SetupLabelForVertex(const unsigned vertex,
                                                    const ClusterLabels label) {
  TernaryClusteringVector::SetupLabelForVertex(vertex, label);
  obj_func_value_increase_relatively_to_first_cluster_[vertex] =
  obj_func_value_increase_relatively_to_second_cluster_[vertex] =
  obj_func_value_increase_relatively_to_third_cluster_[vertex] =
  number_of_neighbours_in_non_clustered_graph_[vertex] = 0;
  for (unsigned long i = 0; i < labels_.size(); i++) {
    if (i == vertex) continue;
    auto i_label = labels_[i];
    auto is_joined = graph_->IsJoined(vertex, i);
    if (i_label != NON_CLUSTERED) {
      if ((i_label == label && !is_joined) || (i_label != label && is_joined)) {
        obj_func_value_on_partially_built_clustering_++;
      }
    } else {
      switch (label) {
        case FIRST_CLUSTER:
          if (is_joined) {
            obj_func_value_increase_relatively_to_second_cluster_[i]++;
            obj_func_value_increase_relatively_to_third_cluster_[i]++;
          } else {
            obj_func_value_increase_relatively_to_first_cluster_[i]++;
          }
          break;
        case SECOND_CLUSTER:
          if (is_joined) {
            obj_func_value_increase_relatively_to_first_cluster_[i]++;
            obj_func_value_increase_relatively_to_third_cluster_[i]++;
          } else {
            obj_func_value_increase_relatively_to_second_cluster_[i]++;
          }
          break;
        case THIRD_CLUSTER:
          if (is_joined) {
            obj_func_value_increase_relatively_to_first_cluster_[i]++;
            obj_func_value_increase_relatively_to_second_cluster_[i]++;
          } else {
            obj_func_value_increase_relatively_to_third_cluster_[i]++;
          }
          break;
        default:throw std::logic_error("unsupported label");
      }
      if (is_joined) {
        num_of_edges_of_non_constructed_graph_--;
        number_of_neighbours_in_non_clustered_graph_[i]--;
      }
    }
  }
}

unsigned BBTernaryClusteringVector::Choose() const {
  unsigned candidate = UINT_MAX;
  int best_dist = INT_MIN;
  for (unsigned long i = 0; i < labels_.size(); i++) {
    if (labels_[i] != NON_CLUSTERED) continue;
    auto tmp_dist = std::min(
        std::min(obj_func_value_increase_relatively_to_first_cluster_[i],
                 obj_func_value_increase_relatively_to_second_cluster_[i]),
        obj_func_value_increase_relatively_to_third_cluster_[i])
        + number_of_neighbours_in_non_clustered_graph_[i];
    if (tmp_dist > best_dist) {
      best_dist = tmp_dist;
      candidate = i;
    }
  }
  return candidate;
}

int BBTernaryClusteringVector::Bound(const unsigned record) const {
  auto result = obj_func_value_on_partially_built_clustering_;
  if (result >= record) {
    return result;
  }
  int u = (int) num_non_clustered_vertices_;
  int delta3 = u * (u - 3) / 6 - num_of_edges_of_non_constructed_graph_;
  result += std::max(0, delta3);
  if (result >= record) {
    return result;
  }
  for (unsigned long i = 0; i < labels_.size(); i++) {
    result += std::min(
        std::min(obj_func_value_increase_relatively_to_first_cluster_[i],
                 obj_func_value_increase_relatively_to_second_cluster_[i]),
        obj_func_value_increase_relatively_to_third_cluster_[i]);
    if (result >= record) {
      return result;
    }
  }
  return result;
}

BBTernaryClusteringVector BBTernaryClusteringVector::Copy() const {
  return BBTernaryClusteringVector(*this);
}