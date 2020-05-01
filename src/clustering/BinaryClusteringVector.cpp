#include <stdexcept>
#include "../../include/clustering/BinaryClusteringVector.hpp"

void BinaryClusteringVector::SetupLabelForVertex(const unsigned vertex, const unsigned label) {
  switch (label) {
    case 0:
      num_vertices_in_first_cluster_++;
      break;
    case 1:
      num_vertices_in_second_cluster_++;
      break;
    default:auto message = "Expected label 0 or 1, actual label = " + std::to_string(label);
      throw std::invalid_argument(message);
  }
  auto cur_label = labels_[vertex];
  switch (cur_label) {
    case -1:
      num_non_clustered_vertices_--;
      break;
    case 0:
      num_vertices_in_first_cluster_--;
      break;
    case 1:
      num_vertices_in_second_cluster_--;
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
  labels_ = std::vector<int>(size, -1);
  num_non_clustered_vertices_ = size;
  num_vertices_in_first_cluster_ = num_vertices_in_second_cluster_ = 0;
}
std::shared_ptr<IClustering> BinaryClusteringVector::GetCopy() const {
  return std::shared_ptr<IClustering>(new BinaryClusteringVector(*this));
}
int BinaryClusteringVector::GetLabel(const unsigned vertex) const {
  return labels_[vertex];
}
bool BinaryClusteringVector::IsNonClustered(const unsigned vertex) const {
  return labels_[vertex] == -1;
}
bool BinaryClusteringVector::IsSameClustered(const unsigned i, const unsigned j) const {
  return labels_[i] == labels_[j];
}
unsigned BinaryClusteringVector::GetNumNonClusteredVertices() const {
  return num_non_clustered_vertices_;
}
unsigned int BinaryClusteringVector::GetNumVerticesByLabel(const unsigned label) const {
  if (label == 0) {
    return num_vertices_in_first_cluster_;
  }
  return num_vertices_in_second_cluster_;
}
