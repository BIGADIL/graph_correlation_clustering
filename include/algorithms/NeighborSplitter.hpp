#pragma once

#include <memory>
#include "../clustering/factories/IClusteringFactory.hpp"
#include "../common/Constants.hpp"
class NeighborSplitter {
 private:
  std::shared_ptr<IClusteringFactory> clustering_factory_;

 public:
  NeighborSplitter(std::shared_ptr<IClusteringFactory> clustering_factory);
  IClusteringPointer SplitGraphByVertex(const IGraph &graph, const unsigned vertex) const;
};



