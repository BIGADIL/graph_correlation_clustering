#pragma once

#include <random>
#include <set>
#include <climits>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <barrier>
#include "IGeneticAlgorithm.hpp"
#include "../../../clustering/factories/TripleClusteringFactory.hpp"
namespace non_strict_3cc {

struct ToMergeCandidate {
  unsigned i = -1;
  unsigned j = -1;
  int record = INT_MAX;
};

class GeneticAlgorithm : public IGeneticAlgorithm {
 private:
  std::shared_ptr<Solution> record_ = nullptr;
  unsigned num_iter_without_record_ = 0;
  std::shared_ptr<IGraph> graph_ = nullptr;
  std::vector<Solution> population_;
  std::vector<Solution> buffer_;
  std::atomic<bool> stop_training_ = false;
  unsigned num_threads_;

  std::unique_ptr<std::barrier<>> barrier_;

  unsigned iterations_;
  unsigned early_stop_num_;
  IClustFactoryPtr factory_;
  unsigned population_size_;
  unsigned tournament_size_;
  double p_mutation_;

  void WorkerLoop(std::vector<Solution> &local_buffer, unsigned max_capacity);
  void ThreadWorker(std::vector<Solution> &local_buffer, unsigned max_capacity, unsigned iteration);
  Solution ShakeUp(const Solution &solution, double p_shake);

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

  IClustPtr CreateClusteringByBases(std::vector<std::set<unsigned>> &bases);

  ToMergeCandidate FindCandidateToMerge(std::vector<std::set<unsigned>> &bases);

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
