#include "../../../../include/solvers/non_strict_two_correlation_clustering/common_functions/NSplitterForNS2CC.hpp"

ns2cc::NSplitterForNS2CC::NSplitterForNS2CC(IClustFactoryPtr clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {

}
IClustPtr ns2cc::NSplitterForNS2CC::SplitGraphByVertex(const IGraph &graph,
                                                       const unsigned vertex) const {
  IClustPtr split_clustering = clustering_factory_->CreateClustering(graph.Size());
  for (unsigned i = 0; i < graph.Size(); i++) {
    if (i == vertex || graph.IsJoined(i, vertex)) {
      split_clustering->SetupLabelForVertex(i, FIRST_CLUSTER);
    } else {
      split_clustering->SetupLabelForVertex(i, SECOND_CLUSTER);
    }
  }
  return split_clustering;
}
