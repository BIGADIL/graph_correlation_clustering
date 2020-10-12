#include <stdexcept>
#include <sstream>
#include "../../include/clustering/BinaryClusteringVector.hpp"

void BinaryClusteringVector::SetupLabelForVertex(const unsigned vertex,
                                                 const ClusterLabels label) {
  switch (label) {
    case FIRST_CLUSTER:num_vertices_in_first_cluster_++;
      break;
    case SECOND_CLUSTER:num_vertices_in_second_cluster_++;
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
    default:throw std::invalid_argument("undefined cur label=" + std::to_string(cur_label));
  }
  labels_[vertex] = label;
}

unsigned BinaryClusteringVector::GetDistanceToGraph(const IGraph &graph) const {
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

BinaryClusteringVector::BinaryClusteringVector(const unsigned size) {
  labels_ = std::vector<ClusterLabels>(size, NON_CLUSTERED);
  num_non_clustered_vertices_ = size;
  num_vertices_in_first_cluster_ = num_vertices_in_second_cluster_ = 0;
}

IClustPtr BinaryClusteringVector::GetCopy() const {
  return std::shared_ptr<IClustering>(new BinaryClusteringVector(*this));
}

ClusterLabels BinaryClusteringVector::GetLabel(const unsigned vertex) const {
  return labels_[vertex];
}

bool BinaryClusteringVector::IsNonClustered(const unsigned vertex) const {
  return labels_[vertex] == NON_CLUSTERED;
}

bool BinaryClusteringVector::IsSameClustered(const unsigned i,
                                             const unsigned j) const {
  return labels_[i] == labels_[j];
}
unsigned BinaryClusteringVector::GetNumNonClusteredVertices() const {
  return num_non_clustered_vertices_;
}

unsigned int BinaryClusteringVector::GetNumVerticesByLabel(const unsigned label) const {
  if (label == ClusterLabels::FIRST_CLUSTER) {
    return num_vertices_in_first_cluster_;
  }
  return num_vertices_in_second_cluster_;
}

std::string BinaryClusteringVector::ToJson() const {
  std::stringstream ss;
  ss << "\"binary clustering vector \": [";
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
