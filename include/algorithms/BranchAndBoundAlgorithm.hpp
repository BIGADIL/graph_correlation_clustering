#pragma once

#include "../clustering/factories/IClusteringFactory.hpp"
#include "../common/Constants.hpp"
#include "../clustering/BoundAndBranchBinaryClusteringVector.hpp"
class BranchAndBoundAlgorithm {
 private:
  std::shared_ptr<IGraph> graph_;
  unsigned record_{0};
  std::shared_ptr<IClustering> best_clustering_ = nullptr;

  void Branch(BoundAndBranchBinaryClusteringVector &clustering);

 public:
  explicit BranchAndBoundAlgorithm() = default;
  std::shared_ptr<IClustering> GetBestClustering(const std::shared_ptr<IGraph>& graph, const std::shared_ptr<IClustering> &clustering_record);
};



