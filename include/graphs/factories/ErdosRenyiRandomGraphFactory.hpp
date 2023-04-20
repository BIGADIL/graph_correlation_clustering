#pragma once

#include <random>

#include "../AdjacencyMatrixGraph.hpp"
#include "IGraphFactory.hpp"

/**
 * Factory creates new random graph by Erdos-Renyi model.
 * https://en.wikipedia.org/wiki/Erdős–Rényi_model
 */
class ErdosRenyiRandomGraphFactory : IGraphFactory {
 private:
  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::uniform_real_distribution<> dis_{0.0, 1.0};
  /**
   * Density of generated graph.
   */
  double density_;

 public:
  ErdosRenyiRandomGraphFactory(const double density);
  IGraphPtr CreateGraph(const unsigned size) override;
};
