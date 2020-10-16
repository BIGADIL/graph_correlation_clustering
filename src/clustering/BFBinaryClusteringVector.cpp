#include <sstream>
#include "../../include/clustering/BFBinaryClusteringVector.hpp"

bool BFBinaryClusteringVector::IsNonClustered(const unsigned int vertex) const {
  throw std::logic_error("not implemented");
}

ClusterLabels BFBinaryClusteringVector::GetLabel(const unsigned int vertex) const {
  throw std::logic_error("not implemented");
}

unsigned BFBinaryClusteringVector::GetDistanceToGraph(const IGraph &graph) const {
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

void BFBinaryClusteringVector::SetupLabelForVertex(const unsigned int vertex, ClusterLabels label) {
  switch (label) {
    case FIRST_CLUSTER:labels_[vertex] = 0;
      break;
    case SECOND_CLUSTER:labels_[vertex] = 1;
      break;
    default:throw std::logic_error("not supported");
  }
}

IClustPtr BFBinaryClusteringVector::GetCopy() const {
  return std::shared_ptr<IClustering>(new BFBinaryClusteringVector(*this));
}

bool BFBinaryClusteringVector::IsSameClustered(const unsigned int i, const unsigned int j) const {
  return labels_[i] == labels_[j];
}

unsigned BFBinaryClusteringVector::GetNumNonClusteredVertices() const {
  throw std::logic_error("not implemented");
}

unsigned int BFBinaryClusteringVector::GetNumVerticesByLabel(ClusterLabels label) const {
  throw std::logic_error("not implemented");
}

std::string BFBinaryClusteringVector::ToJson() const {
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

BFBinaryClusteringVector::BFBinaryClusteringVector(const unsigned int size) {
  labels_ = std::vector<int>(size, 0);
}

IClustPtr BFBinaryClusteringVector::getNextClustering() {
  static auto next_clustering = Copy();
  static std::vector<int> one(labels_.size(), 0);
  one[0] = 1;
  static bool was_first_clustering = false;
  unsigned base = 2;
  if (!was_first_clustering) {
    was_first_clustering = true;
    return next_clustering.GetCopy();
  }
  for (unsigned i = 0; i < labels_.size(); ++i) {
    next_clustering.labels_[i] += one[i];
    if (i != labels_.size() - 1) {
      next_clustering.labels_[i + 1] += next_clustering.labels_[i] / base;
    }
    next_clustering.labels_[i] %= base;
  }
  if (next_clustering.labels_[labels_.size() - 1] == 1) {
    return nullptr;
  }
  return next_clustering.GetCopy();
}

BFBinaryClusteringVector BFBinaryClusteringVector::Copy() const {
  return BFBinaryClusteringVector(*this);
}

