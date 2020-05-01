#include "../../include/algorithms/NeighborSplitter.hpp"
NeighborSplitter::NeighborSplitter(std::shared_ptr<IClusteringFactory> clustering_factory) :
    clustering_factory_(std::move(clustering_factory)) {

}
IClusteringPointer NeighborSplitter::SplitGraphByVertex(const IGraph &graph, const unsigned vertex) const {
  IClusteringPointer split_clustering = clustering_factory_->CreateClustering(graph.Size());
  for (unsigned i = 0; i < graph.Size(); i++) {
    if (i == vertex || graph.IsJoined(i, vertex)) {
      split_clustering->SetupLabelForVertex(i, 0);
    } else {
      split_clustering->SetupLabelForVertex(i, 1);
    }
  }
  return split_clustering;
}
