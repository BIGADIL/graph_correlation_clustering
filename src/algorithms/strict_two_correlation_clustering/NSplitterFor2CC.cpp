#include "../../../include/algorithms/strict_two_correlation_clustering/NSplitterFor2CC.hpp"

NSplitterForS2CC::NSplitterForS2CC(IClustFactoryPtr clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {

}
IClustPtr NSplitterForS2CC::SplitGraphByVertex(const IGraph &graph,
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
