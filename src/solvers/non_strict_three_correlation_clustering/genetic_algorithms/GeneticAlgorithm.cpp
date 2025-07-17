
#include <algorithm>
#include <set>
#include <iostream>
#include <utility>
#include "../../../../include/solvers/non_strict_three_correlation_clustering/genetic_algorithms/GeneticAlgorithm.hpp"
#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithLocalSearch.hpp"

non_strict_3cc::GeneticAlgorithm::GeneticAlgorithm(unsigned iterations,
                                                   unsigned early_stop_num,
                                                   IClustFactoryPtr factory,
                                                   unsigned population_size,
                                                   unsigned tournament_size,
                                                   double p_mutation) :
    iterations_(iterations),
    early_stop_num_(early_stop_num),
    factory_(std::move(factory)),
    population_size_(population_size),
    tournament_size_(tournament_size),
    p_mutation_(p_mutation) {

}

Solution non_strict_3cc::GeneticAlgorithm::Mutation(const Solution &solution, double p_mutation) {
  auto copy = solution.clustering->GetCopy();
  std::uniform_int_distribution<> dis_n(0, solution.clustering->Size() - 1);
  for (unsigned i = 0; i < graph_->Size(); i++) {
    auto p = dis(gen);
    if (p > p_mutation) {
      continue;
    }
    auto j = dis_n(gen);
    auto label_i = copy->GetLabel(i);
    auto label_j = copy->GetLabel(j);
    copy->SetupLabelForVertex(i, label_j);
    copy->SetupLabelForVertex(j, label_i);
  }
  return {copy->GetDistanceToGraph(*graph_), copy};
}

std::pair<Solution, Solution> non_strict_3cc::GeneticAlgorithm::Crossover(const Solution &x,
                                                                          const Solution &y) {
  std::vector<Solution> solutions;
  solutions.emplace_back(x);
  solutions.emplace_back(y);

  auto child_1 = factory_->CreateClustering(graph_->Size());
  auto child_2 = factory_->CreateClustering(graph_->Size());

  for (unsigned i = 0; i < graph_->Size(); i++) {
    if (dis(gen) > 0.5) {
      child_1->SetupLabelForVertex(i, x.clustering->GetLabel(i));
      child_2->SetupLabelForVertex(i, y.clustering->GetLabel(i));
    } else {
      child_1->SetupLabelForVertex(i, y.clustering->GetLabel(i));
      child_2->SetupLabelForVertex(i, x.clustering->GetLabel(i));
    }
  }
  solutions.emplace_back(child_1->GetDistanceToGraph(*graph_), child_1);
  solutions.emplace_back(child_2->GetDistanceToGraph(*graph_), child_2);
  std::sort(solutions.begin(), solutions.end());
  return {solutions[0], solutions[1]};
}

Solution non_strict_3cc::GeneticAlgorithm::Selection(std::vector<Solution> population) {
  std::vector<Solution> solutions;
  std::uniform_int_distribution<> dis_n(0, population.size() - 1);
  for (unsigned i = 0; i < tournament_size_; i++) {
    solutions.emplace_back(population[dis_n(gen)]);
  }
  Solution best = solutions[0];
  for (unsigned i = 1; i < solutions.size(); i++) {
    if (solutions[i].distance < best.distance) {
      best = solutions[i];
    }
  }
  return best;
}

std::vector<Solution> non_strict_3cc::GeneticAlgorithm::GenerateInitPopulation(unsigned int population_size) {
  auto generator = non_strict_3cc::TwoVerticesNeighborhoodWithLocalSearch(8, factory_);
  auto solutions = generator.getAllSolutions(*graph_);
  std::vector<Solution> result;
  for (auto &it: solutions) {
    result.emplace_back(it);
  }
  return result;
}

void non_strict_3cc::GeneticAlgorithm::OnIterationBegin(unsigned int iteration) {
  buffer_.clear();
}

void non_strict_3cc::GeneticAlgorithm::OnIterationEnd(unsigned int iteration) {
  population_.clear();
  for (auto &it: buffer_) {
    population_.emplace_back(it);
  }

  Solution best = population_[0];
  for (unsigned i = 1; i < population_.size(); i++) {
    if (population_[i].distance < best.distance) {
      best = population_[i];
    }
  }

  if (best.distance >= record_->distance) {
    num_iter_without_record_++;
  } else {
    record_ = std::make_shared<Solution>(best.getCopy());
    num_iter_without_record_ = 0;
  }

  if (num_iter_without_record_ == early_stop_num_) {
    stop_training_ = true;
  }
  if (iteration % 10 == 0 && !stop_training_) {
    population_.clear();
    for (auto &it: buffer_) {
      auto x = it;
      for (unsigned j = 0; j < 10; j++) {
        x = Mutation(x, 1e-1);
      }
      population_.emplace_back(x);
    }
  }
}

Solution non_strict_3cc::GeneticAlgorithm::Train(std::shared_ptr<IGraph> graph) {
  record_ = nullptr;
  graph_ = graph;
  buffer_ = GenerateInitPopulation(population_size_);
  population_.clear();
  for (auto &it: buffer_) {
    if (!IsIn(it, population_)) {
      population_.emplace_back(it);
    }
  }

  for (auto &it: population_) {
    if (record_ == nullptr || record_->distance > it.distance) {
      record_ = std::make_shared<Solution>(it);
    }
  }

  num_iter_without_record_ = 0;
  stop_training_ = false;

  for (unsigned i = 0; i < iterations_; i++) {
    OnIterationBegin(i);
    while (buffer_.size() < population_size_) {
      auto x = Selection(population_);
      auto y = Selection(population_);
      auto pair = Crossover(x, y);
      if (!(pair.first == x) && !(pair.first == y)) {
        std::cout << "";
      }

      x = Mutation(pair.first, p_mutation_);

      if (buffer_.size() < population_size_ && !IsIn(x, buffer_)) {
        buffer_.emplace_back(x);
      }
      if (buffer_.size() < population_size_ && IsIn(y, buffer_)) {
        buffer_.emplace_back(y);
      }
    }
    OnIterationEnd(i);
    if (stop_training_) {
      break;
    }
  }
  return *record_;
}
