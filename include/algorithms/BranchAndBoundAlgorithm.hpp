#pragma once

#include "../clustering/factories/IClusteringFactory.hpp"
#include "../common/Constants.hpp"
#include "../clustering/BoundAndBranchBinaryClusteringVector.hpp"
class BranchAndBoundAlgorithm {
 private:
  std::shared_ptr<IGraph> graph_;
  unsigned record_{0};

  void Branch(const BoundAndBranchBinaryClusteringVector &clustering);

 public:
  explicit BranchAndBoundAlgorithm() = default;
  unsigned GetBestClustering(const std::shared_ptr<IGraph>& graph, int record);
};



