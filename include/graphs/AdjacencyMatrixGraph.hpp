#pragma once

#include <vector>

#include "IGraph.hpp"

/**
 * Representation of a graph as an adjacency matrix.
 */
class AdjacencyMatrixGraph : public IGraph {
 private:
  std::vector<std::vector<bool>> adjacency_matrix_;

 public:
  AdjacencyMatrixGraph() = delete;
  explicit AdjacencyMatrixGraph(const std::vector<std::vector<bool>> &adjacency_matrix) : adjacency_matrix_(
      adjacency_matrix) {}
  [[nodiscard]] bool IsJoined(const unsigned i,
                              const unsigned j) const override { return adjacency_matrix_[i][j]; }
  [[nodiscard]] unsigned Size() const override { return adjacency_matrix_.size(); }
  [[nodiscard]] std::string ToJson() const override;

  bool operator==(IGraph &other) const override;
};
