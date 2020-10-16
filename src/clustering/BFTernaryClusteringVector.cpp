#include <sstream>
#include "../../include/clustering/BFTernaryClusteringVector.hpp"

bool BFTernaryClusteringVector::IsNonClustered(const unsigned int vertex) const {
  throw std::logic_error("not implemented");
}

ClusterLabels BFTernaryClusteringVector::GetLabel(const unsigned int vertex) const {
  throw std::logic_error("not implemented");
}

unsigned BFTernaryClusteringVector::GetDistanceToGraph(const IGraph &graph) const {
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

void BFTernaryClusteringVector::SetupLabelForVertex(const unsigned int vertex, ClusterLabels label) {
  switch (label) {
    case FIRST_CLUSTER:labels_[vertex] = 0;
      break;
    case SECOND_CLUSTER:labels_[vertex] = 1;
      break;
    case THIRD_CLUSTER:labels_[vertex] = 2;
      break;
    default:throw std::logic_error("not supported");
  }
}

IClustPtr BFTernaryClusteringVector::GetCopy() const {
  return std::shared_ptr<IClustering>(new BFTernaryClusteringVector(*this));
}

bool BFTernaryClusteringVector::IsSameClustered(const unsigned int i, const unsigned int j) const {
  return labels_[i] == labels_[j];
}

unsigned BFTernaryClusteringVector::GetNumNonClusteredVertices() const {
  throw std::logic_error("not implemented");
}

unsigned int BFTernaryClusteringVector::GetNumVerticesByLabel(ClusterLabels label) const {
  throw std::logic_error("not implemented");
}

std::string BFTernaryClusteringVector::ToJson() const {
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

BFTernaryClusteringVector::BFTernaryClusteringVector(const unsigned int size) {
  labels_ = std::vector<int>(size, 0);
}

IClustPtr BFTernaryClusteringVector::getNextClustering() {
  static auto next_clustering = Copy();
  static std::vector<int> one(labels_.size(), 0);
  one[0] = 1;
  static bool was_first_clustering = false;
  unsigned base = 3;
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

BFTernaryClusteringVector BFTernaryClusteringVector::Copy() const {
  return BFTernaryClusteringVector(*this);
}