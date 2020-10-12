#pragma once

#include <vector>
#include "IGraph.hpp"
class AdjacencyMatrixGraph : public IGraph {
 private:
  std::vector<std::vector<bool>> adjacency_matrix_;

 public:
  AdjacencyMatrixGraph() = delete;
  AdjacencyMatrixGraph(const std::vector<std::vector<bool>> &adjacency_matrix) : adjacency_matrix_(adjacency_matrix) {}
  bool IsJoined(const unsigned i,
                const unsigned j) const override { return adjacency_matrix_[i][j]; }
  unsigned Size() const override { return adjacency_matrix_.size(); }
  std::string ToJson() const override;
};
