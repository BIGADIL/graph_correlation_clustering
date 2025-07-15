#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/NeighborSplitter.hpp"

IClustPtr non_strict_3cc::NeighborSplitter::BuildFirstCluster(const IGraph &graph,
                                                              const unsigned vertex) const {
  auto result = clustering_factory_->CreateClustering(graph.Size());
  for (unsigned i = 0; i < graph.Size(); i++) {
    if (i == vertex || graph.IsJoined(i, vertex)) {
      result->SetupLabelForVertex(i, FIRST_CLUSTER);
    }
  }
  return result;
}

IClustPtr non_strict_3cc::NeighborSplitter::BuildSecondAndThirdClusters(const IGraph &graph,
                                                                        const IClustPtr &init_clustering,
                                                                        const unsigned vertex) {
  auto result = init_clustering->GetCopy();
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if (i != vertex && result->GetLabel(i) == FIRST_CLUSTER) continue;
    if (i == vertex || graph.IsJoined(i, vertex)) {
      result->SetupLabelForVertex(i, SECOND_CLUSTER);
    } else {
      result->SetupLabelForVertex(i, THIRD_CLUSTER);
    }
  }
  return result;
}

IClustPtr non_strict_3cc::NeighborSplitter::SplitGraphByTwoVertices(const IGraph &graph,
                                                                    const unsigned first_vertex,
                                                                    const unsigned second_vertex) const {
  auto result = clustering_factory_->CreateClustering(graph.Size());
  for (unsigned i = 0; i < graph.Size(); ++i) {
    if (i == first_vertex) {
      result->SetupLabelForVertex(i, FIRST_CLUSTER);
    } else if (i == second_vertex) {
      result->SetupLabelForVertex(i, SECOND_CLUSTER);
    } else {
      if (graph.IsJoined(i, first_vertex)) {
        result->SetupLabelForVertex(i, FIRST_CLUSTER);
      } else if (graph.IsJoined(i, second_vertex)) {
        result->SetupLabelForVertex(i, SECOND_CLUSTER);
      } else {
        result->SetupLabelForVertex(i, THIRD_CLUSTER);
      }
    }
  }
  return result;
}

non_strict_3cc::NeighborSplitter::NeighborSplitter(IClustFactoryPtr clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {

}
