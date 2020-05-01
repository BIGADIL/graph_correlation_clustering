#pragma once

#include "BinaryClusteringVector.hpp"

class BoundAndBranchBinaryClusteringVector : public BinaryClusteringVector {
 private:
  unsigned obj_func_value_on_partially_built_clustering_{0};
  int num_of_edges_of_non_constructed_graph_{0};
  std::vector<int> obj_func_value_increase_relatively_to_first_cluster_;
  std::vector<int> obj_func_value_increase_relatively_to_second_cluster_;
  std::vector<int> number_of_neighbours_in_graph_;

  std::shared_ptr<IGraph> graph_;

 protected:
  BoundAndBranchBinaryClusteringVector() = delete;
  BoundAndBranchBinaryClusteringVector(const BoundAndBranchBinaryClusteringVector &) = default;
  BoundAndBranchBinaryClusteringVector(const BoundAndBranchBinaryClusteringVector &&) = delete;
  BoundAndBranchBinaryClusteringVector &operator=(const BoundAndBranchBinaryClusteringVector &) = delete;
  BoundAndBranchBinaryClusteringVector &operator=(const BoundAndBranchBinaryClusteringVector &&) = delete;

 public:
  BoundAndBranchBinaryClusteringVector(const unsigned size, const std::shared_ptr<IGraph> &graph);
  void SetupLabelForVertex(const unsigned vertex, const ClusterLabels label) override;
  unsigned Choose() const;
  int Bound(const unsigned record) const;

  BoundAndBranchBinaryClusteringVector copy() const;
};



