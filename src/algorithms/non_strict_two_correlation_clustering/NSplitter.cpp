#include "../../../include/algorithms/non_strict_two_correlation_clustering/NSplitter.hpp"
NSplitter::NSplitter(IClustFactoryPtr clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {

}
IClustPtr NSplitter::SplitGraphByVertex(const IGraph &graph,
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
