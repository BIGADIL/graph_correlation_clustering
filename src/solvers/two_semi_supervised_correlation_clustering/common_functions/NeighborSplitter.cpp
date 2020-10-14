#include "../../../../include/solvers/two_semi_supervised_correlation_clustering/common_functions/NeighborSplitter.hpp"

semi_supervised_2cc::NeighborSplitter::NeighborSplitter(IClustFactoryPtr clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {
}

std::vector<IClustPtr> semi_supervised_2cc::NeighborSplitter::SplitGraphByVertex(const IGraph &graph,
                                                                                 unsigned int vertex,
                                                                                 unsigned int first_cluster_vertex,
                                                                                 unsigned int second_cluster_vertex) const {
  std::vector<IClustPtr> result;
  if (vertex == first_cluster_vertex) {
    IClustPtr split_clustering = clustering_factory_->CreateClustering(graph.Size());
    for (unsigned i = 0; i < graph.Size(); ++i) {
      if ((i == vertex || graph.IsJoined(i, vertex)) && i != second_cluster_vertex) {
        split_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
      } else {
        split_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
      }
    }
    result.push_back(split_clustering);
  } else if (vertex == second_cluster_vertex) {
    IClustPtr split_clustering = clustering_factory_->CreateClustering(graph.Size());
    for (unsigned i = 0; i < graph.Size(); ++i) {
      if ((i == vertex || graph.IsJoined(i, vertex)) && i != first_cluster_vertex) {
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
      if (i == first_cluster_vertex) {
        first_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
        second_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
      } else if (i == second_cluster_vertex) {
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
