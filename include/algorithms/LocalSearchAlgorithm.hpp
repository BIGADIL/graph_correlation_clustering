#pragma once

#include <memory>
#include "../clustering/factories/IClusteringFactory.hpp"
#include "../common/Constants.hpp"
class LocalSearchAlgorithm {
 private:
  std::shared_ptr<IClusteringFactory> clustering_factory_;

  int ComputeLocalImprovement(const IGraph &graph,
                              const IClusteringPointer &cur_clustering,
                              const unsigned vertex) const;

 public:
  LocalSearchAlgorithm(std::shared_ptr<IClusteringFactory> clustering_factory);
  IClusteringPointer ComputeLocalOptimum(const IGraph &graph, const IClusteringPointer &cur_clustering) const;
};



