#include "../../../include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp"

std::shared_ptr<IGraph> ErdosRenyiRandomGraphFactory::CreateGraph(const unsigned size) {
  std::vector<std::vector<bool>> adjacency_matrix(size);
  for (auto &row: adjacency_matrix) {
    row = std::vector<bool>(size);
  }
  for (unsigned long i = 0; i < size; i++) {
    for (unsigned long j = 0; j < i; j++) {
      if (dis_(gen_) < density_) {
        adjacency_matrix[i][j] = adjacency_matrix[j][i] = true;
      }
    }
  }

  return std::shared_ptr<IGraph>(new AdjacencyMatrixGraph(adjacency_matrix));
}
ErdosRenyiRandomGraphFactory::ErdosRenyiRandomGraphFactory(const double density) : density_(density) {

}
