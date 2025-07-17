#pragma once

#include <vector>
#include <unordered_set>

#include "../../../clustering/IClustering.hpp"

namespace non_strict_3cc {
class IGeneticAlgorithm {
  virtual Solution Mutation(const Solution &solution, double p_mutation) = 0;
  virtual std::pair<Solution, Solution> Crossover(const Solution &x, const Solution &y) = 0;
  virtual Solution Selection(std::vector<Solution> population) = 0;
  virtual std::vector<Solution> GenerateInitPopulation(unsigned population_size) = 0;
  virtual void OnIterationBegin(unsigned iteration) = 0;
  virtual void OnIterationEnd(unsigned iteration) = 0;
  virtual Solution Train(std::shared_ptr<IGraph> graph) = 0;
};
}
