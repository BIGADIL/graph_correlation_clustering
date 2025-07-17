
#include <algorithm>
#include <set>
#include <iostream>
#include <utility>
#include <climits>
#include <map>
#include "../../../../include/solvers/non_strict_three_correlation_clustering/genetic_algorithms/GeneticAlgorithm.hpp"
#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithLocalSearch.hpp"
#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/LocalSearch.hpp"

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
    int plus[3];
    plus[0] = plus[1] = plus[2] = 0;
    for (unsigned j = 0; j < graph_->Size(); j++) {
      if (i == j) {
        continue;
      }
      auto imp = LocalSearch::ComputeLocalImprovement(*graph_, copy, i);
      ClusterLabels label = NON_CLUSTERED;
      int best_imp = INT_MIN;
      for (int k = 0; k < 3; k++) {
        if (k == copy->GetLabel(i)) {
          continue;
        }
        if (imp[k] > best_imp) {
          best_imp = imp[k];
          if (k == 0) {
            label = FIRST_CLUSTER;
          } else if (k == 1) {
            label = SECOND_CLUSTER;
          } else {
            label = THIRD_CLUSTER;
          }
        }
      }
      copy->SetupLabelForVertex(i, label);
    }
  }
  return {copy->GetDistanceToGraph(*graph_), copy};
}

std::vector<Solution> non_strict_3cc::GeneticAlgorithm::Crossover(const Solution &x,
                                                                  const Solution &y) {
  std::vector<Solution> solutions;
  solutions.emplace_back(x);
  solutions.emplace_back(y);

  std::map<int, std::set<unsigned>> clusters_x;
  std::map<int, std::set<unsigned>> clusters_y;

  for (unsigned i = 0; i < graph_->Size(); i++) {
    auto label_x = x.clustering->GetLabel(i);
    auto [it_x, inserted_x] = clusters_x.try_emplace((int) label_x, std::set<unsigned>{});
    it_x->second.insert(i);
    auto label_y = y.clustering->GetLabel(i);
    auto [it_y, inserted_y] = clusters_y.try_emplace((int) label_y, std::set<unsigned>{});
    it_y->second.insert(i);
  }

  std::vector<std::set<unsigned>> bases;
  for (auto &[key_x, value_x]: clusters_x) {
    for (auto &[key_y, value_y]: clusters_y) {
      std::set<unsigned> intersect;
      std::set_intersection(value_x.begin(),
                            value_x.end(),
                            value_y.begin(),
                            value_y.end(),
                            std::inserter(intersect, intersect.begin()));
      bases.emplace_back(intersect);
    }
  }

  bases.erase(
      std::remove_if(
          bases.begin(),
          bases.end(),
          [](const std::set<unsigned> &s) { return s.empty(); }
      ),
      bases.end()
  );

  while (bases.size() > 3) {
    unsigned to_merge_1 = 0;
    unsigned to_merge_2 = 1;
    int record = 0;
    std::set<unsigned> set0 = bases[0];
    std::set<unsigned> set1 = bases[1];
    for (auto &it1: set0) {
      for (auto &it2: set1) {
        auto is_joined = graph_->IsJoined(it1, it2);
        if (is_joined) {
          record--;
        } else {
          record++;
        }
      }
    }

    for (unsigned i = 0; i < bases.size(); i++) {
      for (unsigned j = i + 1; j < bases.size(); j++) {
        if (i == 0 && j == 1) continue;
        int local_record = 0;
        std::set<unsigned> set0 = bases[i];
        std::set<unsigned> set1 = bases[j];
        for (auto &it1: set0) {
          for (auto &it2: set1) {
            auto is_joined = graph_->IsJoined(it1, it2);
            if (is_joined) {
              local_record--;
            } else {
              local_record++;
            }
          }
        }
        if (local_record < record) {
          to_merge_1 = i;
          to_merge_2 = j;
          record = local_record;
        }
      }
    }

    bases[to_merge_1].insert(bases[to_merge_2].begin(), bases[to_merge_2].end());
    bases.erase(bases.begin() + to_merge_2);
  }

  auto child = factory_->CreateClustering(graph_->Size());
  for (unsigned i = 0; i < bases.size(); i++) {
    auto base = bases[i];
    for (auto &it: base) {
      if (i == 0) {
        child->SetupLabelForVertex(it, FIRST_CLUSTER);
      } else if (i == 1) {
        child->SetupLabelForVertex(it, SECOND_CLUSTER);
      } else {
        child->SetupLabelForVertex(it, THIRD_CLUSTER);
      }
    }
  }
  solutions.emplace_back(child->GetDistanceToGraph(*graph_), child);
  std::sort(solutions.begin(), solutions.end());
  return {solutions[0]};
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
  if (iteration % 50 == 0 && !stop_training_) {
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
      for (auto &it: pair) {
        x = Mutation(it, p_mutation_);
        buffer_.emplace_back(x);
      }
    }
    OnIterationEnd(i);
    if (stop_training_) {
      break;
    }
  }
  return *record_;
}
