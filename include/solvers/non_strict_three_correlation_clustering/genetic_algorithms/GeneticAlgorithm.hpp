#pragma once

#include <random>
#include <set>
#include "IGeneticAlgorithm.hpp"
#include "../../../clustering/factories/TripleClusteringFactory.hpp"
namespace non_strict_3cc {
class GeneticAlgorithm: public IGeneticAlgorithm {
 private:
  std::shared_ptr<Solution> record_ = nullptr;
  unsigned num_iter_without_record_ = 0;
  std::shared_ptr<IGraph> graph_ = nullptr;
  std::vector<Solution> population_;
  std::vector<Solution> buffer_;
  bool stop_training_ = false;

  std::random_device rd_;
  std::default_random_engine gen{rd_()};
  std::uniform_real_distribution<> dis;

  unsigned iterations_;
  unsigned early_stop_num_;
  IClustFactoryPtr factory_;
  unsigned population_size_;
  unsigned tournament_size_;
  double p_mutation_;


 public:
  GeneticAlgorithm() = delete;
  GeneticAlgorithm(const GeneticAlgorithm &) = delete;
  GeneticAlgorithm(const GeneticAlgorithm &&) = delete;
  GeneticAlgorithm &operator=(const GeneticAlgorithm &) = delete;
  GeneticAlgorithm &operator=(const GeneticAlgorithm &&) = delete;
  GeneticAlgorithm(unsigned iterations,
                   unsigned early_stop_num,
                   IClustFactoryPtr factory,
                   unsigned population_size,
                   unsigned tournament_size,
                   double p_mutation);

  Solution Mutation(const Solution &solution, double p_mutation) override;
  std::vector<Solution> Crossover(const Solution &x, const Solution &y) override;
  Solution Selection(std::vector<Solution> population) override;
  std::vector<Solution> GenerateInitPopulation(unsigned population_size) override;
  void OnIterationBegin(unsigned iteration) override;
  void OnIterationEnd(unsigned iteration) override;
  Solution Train(std::shared_ptr<IGraph> graph) override;

  IClustPtr CreateClusteringByBases(std::vector<std::set<unsigned>> bases);

  static bool IsIn(Solution &el, std::vector<Solution> &collection) {
    for (auto &it: collection) {
      if (it == el) {
        return true;
      }
    }
    return false;
  }
};
}
