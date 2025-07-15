#include <algorithm>

#include "../../../../include/solvers/two_set_semi_supervised_correlation_clustering/common_functions/NeighborSplitter.hpp"

set_semi_supervised_2cc::NeighborSplitter::NeighborSplitter(IClustFactoryPtr clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {
}

std::vector<IClustPtr> set_semi_supervised_2cc::NeighborSplitter::SplitGraphByVertex(const IGraph &graph,
                                                                                     unsigned int vertex,
                                                                                     const std::vector<unsigned> &first_cluster_vertices,
                                                                                     const std::vector<unsigned> &second_cluster_vertices) const {
  std::vector<IClustPtr> result;
  if (IsVertexInSet(vertex, first_cluster_vertices)) {
    IClustPtr split_clustering = clustering_factory_->CreateClustering(graph.Size());
    for (unsigned i = 0; i < graph.Size(); ++i) {
      auto could_be_in_first_cluster =
          i == vertex || IsVertexInSet(i, first_cluster_vertices) || graph.IsJoined(i, vertex);
      if (could_be_in_first_cluster && !IsVertexInSet(i, second_cluster_vertices)) {
        split_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
      } else {
        split_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
      }
    }
    result.push_back(split_clustering);
  } else if (IsVertexInSet(vertex, second_cluster_vertices)) {
    IClustPtr split_clustering = clustering_factory_->CreateClustering(graph.Size());
    for (unsigned i = 0; i < graph.Size(); ++i) {
      auto could_be_in_second_cluster =
          i == vertex || IsVertexInSet(i, second_cluster_vertices) || graph.IsJoined(i, vertex);
      if (could_be_in_second_cluster && !IsVertexInSet(i, first_cluster_vertices)) {
        split_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
      } else {
        split_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
      }
    }
    result.push_back(split_clustering);
  } else {
    IClustPtr first_clustering = clustering_factory_->CreateClustering(graph.Size());
    IClustPtr second_clustering = clustering_factory_->CreateClustering(graph.Size());
    for (unsigned i = 0; i < graph.Size(); ++i) {
      if (IsVertexInSet(i, first_cluster_vertices)) {
        first_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
        second_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
      } else if (IsVertexInSet(i, second_cluster_vertices)) {
        first_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
        second_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
      } else {
        if (i == vertex || graph.IsJoined(i, vertex)) {
          first_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
          second_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
        } else {
          first_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
          second_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
        }
      }
    }
    result.push_back(first_clustering);
    result.push_back(second_clustering);
  }
  return result;
}

bool set_semi_supervised_2cc::NeighborSplitter::IsVertexInSet(const unsigned int vertex,
                                                              const std::vector<unsigned int> &vertices_set) {
  return std::find(vertices_set.begin(), vertices_set.end(), vertex) != vertices_set.end();
}
