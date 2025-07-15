#include <sstream>
#include "../../include/clustering/TripleClusteringVector.hpp"

void TripleClusteringVector::SetupLabelForVertex(const unsigned vertex,
                                                 const ClusterLabels label) {
  switch (label) {
    case FIRST_CLUSTER:num_vertices_in_first_cluster_++;
      break;
    case SECOND_CLUSTER:num_vertices_in_second_cluster_++;
      break;
    case THIRD_CLUSTER:num_vertices_in_third_cluster_++;
      break;
    default:auto message = "Expected label 0 or 1, actual label = " + std::to_string(label);
      throw std::invalid_argument(message);
  }
  auto cur_label = labels_[vertex];
  switch (cur_label) {
    case NON_CLUSTERED:num_non_clustered_vertices_--;
      break;
    case FIRST_CLUSTER:num_vertices_in_first_cluster_--;
      break;
    case SECOND_CLUSTER:num_vertices_in_second_cluster_--;
      break;
    case THIRD_CLUSTER:num_vertices_in_third_cluster_--;
      break;
    default:throw std::invalid_argument("undefined cur label=" + std::to_string(cur_label));
  }
  labels_[vertex] = label;
}

unsigned TripleClusteringVector::GetDistanceToGraph(const IGraph &graph) const {
  if (graph.Size() != labels_.size()) {
    auto message = "Graph size must be equal to labels length. Graph size = " + std::to_string(graph.Size())
        + "; labels length = " + std::to_string(labels_.size());
    throw std::invalid_argument(message);
  }
  unsigned distance = 0;
  for (unsigned i = 0; i < graph.Size(); i++) {
    for (unsigned j = i + 1; j < graph.Size(); j++) {
      if ((labels_[i] != labels_[j] && graph.IsJoined(i, j)) || (labels_[i] == labels_[j] && !graph.IsJoined(i, j))) {
        distance++;
      }
    }
  }
  return distance;
}

TripleClusteringVector::TripleClusteringVector(const unsigned size) {
  labels_ = std::vector<ClusterLabels>(size, NON_CLUSTERED);
  num_non_clustered_vertices_ = size;
  num_vertices_in_first_cluster_ = num_vertices_in_second_cluster_ = num_vertices_in_third_cluster_ = 0;
}

IClustPtr TripleClusteringVector::GetCopy() const {
  return std::shared_ptr<IClustering>(new TripleClusteringVector(*this));
}

ClusterLabels TripleClusteringVector::GetLabel(const unsigned vertex) const {
  return labels_[vertex];
}

bool TripleClusteringVector::IsNonClustered(const unsigned vertex) const {
  return labels_[vertex] == NON_CLUSTERED;
}

bool TripleClusteringVector::IsSameClustered(const unsigned i,
                                             const unsigned j) const {
  return labels_[i] == labels_[j];
}
unsigned TripleClusteringVector::GetNumNonClusteredVertices() const {
  return num_non_clustered_vertices_;
}

unsigned int TripleClusteringVector::GetNumVerticesByLabel(const ClusterLabels label) const {
  switch (label) {
    case FIRST_CLUSTER:
      return num_vertices_in_first_cluster_;
    case SECOND_CLUSTER:
      return num_vertices_in_second_cluster_;
    case THIRD_CLUSTER:
      return num_vertices_in_third_cluster_;
    default:
      throw std::invalid_argument("undefined cur label=" + std::to_string(label));
  }
}

std::string TripleClusteringVector::ToJson() const {
  std::stringstream ss;
  ss << "\"ternary clustering vector \": [";
  unsigned row_idx = 0;
  for (const auto &label: labels_) {
    row_idx++;
    if (row_idx == labels_.size()) {
      ss << label << "]";
    } else {
      ss << label << ",";
    }
  }
  return ss.str();
}
unsigned TripleClusteringVector::Size() const {
  return labels_.size();
}
