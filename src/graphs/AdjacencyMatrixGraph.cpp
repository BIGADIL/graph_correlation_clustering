

#include <string>
#include <sstream>
#include "../../include/graphs/AdjacencyMatrixGraph.hpp"

std::string AdjacencyMatrixGraph::ToJson() const {
  std::stringstream ss;
  ss << "\"graph\": " << "[" << std::endl;
  unsigned row_idx = 0;
  for(const auto &row: adjacency_matrix_) {
    row_idx++;
    ss << "[";
    unsigned elem_idx = 0;
    for (const auto &elem: row) {
      elem_idx++;
      ss << elem;
      if (elem_idx != row.size()) {
        ss << ",";
      }
    }
    if (row_idx != adjacency_matrix_.size()) {
      ss << "],";
    } else {
      ss << "]";
    }
    ss << std::endl;
  }
  ss << "]";
  return ss.str();
}

