#include "../../../../include/solvers/strict_two_correlation_clustering/common_functions/NeighborSplitter.hpp"

strict_2cc::NeighborSplitter::NeighborSplitter(IClustFactoryPtr clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {

}
IClustPtr strict_2cc::NeighborSplitter::SplitGraphByVertex(const IGraph &graph,
                                                           const unsigned vertex,
                                                           const unsigned opposite_vertex) const {
  IClustPtr split_clustering = clustering_factory_->CreateClustering(graph.Size());
  for (unsigned i = 0; i < graph.Size(); i++) {
    if ((i == vertex || graph.IsJoined(i, vertex)) && i != opposite_vertex) {
      split_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
    } else {
      split_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
    }
  }
  return split_clustering;
}
