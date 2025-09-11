
#include <algorithm>
#include <set>
#include <iostream>
#include <utility>
#include <climits>
#include <map>
#include <thread>
#include "../../../../include/solvers/non_strict_two_correlation_clustering/genetic_algorithms/GeneticAlgorithm.hpp"
#include "../../../../include/solvers/non_strict_two_correlation_clustering/common_functions/LocalSearch.hpp"
#include "../../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithOneLocalSearch.hpp"

non_strict_2cc::GeneticAlgorithm::GeneticAlgorithm(unsigned iterations,
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
  num_threads_ = 8;
}

Solution non_strict_2cc::GeneticAlgorithm::Mutation(const Solution &solution, double p_mutation) {
  auto copy = solution.clustering->GetCopy();
  auto lo = LocalSearch::ComputeLocalOptimum(*graph_, copy);
  return {lo->GetDistanceToGraph(*graph_), lo};
}

Solution non_strict_2cc::GeneticAlgorithm::ShakeUp(const Solution &solution, double p_shake) {
  auto copy = solution.clustering->GetCopy();
  std::random_device rd_;
  std::default_random_engine gen{rd_()};
  std::uniform_real_distribution<> dis;
  for (unsigned i = 0; i < graph_->Size(); i++) {
    auto p = dis(gen);
    if (p > p_shake) {
      continue;
    }
    auto label = copy->GetLabel(i);
    if (label == FIRST_CLUSTER) {
      copy->SetupLabelForVertex(i, SECOND_CLUSTER);
    } else {
      copy->SetupLabelForVertex(i, FIRST_CLUSTER);
    }
  }
  return {copy->GetDistanceToGraph(*graph_), copy};
}

std::vector<Solution> non_strict_2cc::GeneticAlgorithm::Crossover(const Solution &x,
                                                                  const Solution &y) {

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

  while (bases.size() > 2) {
    auto candidate = FindCandidateToMerge(bases);
    bases[candidate.i].insert(bases[candidate.j].begin(), bases[candidate.j].end());
    bases.erase(bases.begin() + candidate.j);
  }

  while (true) {
    auto candidate = FindCandidateToMerge(bases);
    if (candidate.record < 0) {
      bases[candidate.i].insert(bases[candidate.j].begin(), bases[candidate.j].end());
      bases.erase(bases.begin() + candidate.j);
    } else {
      break;
    }
  }

  auto child = CreateClusteringByBases(bases);
  return {Solution(child->GetDistanceToGraph(*graph_), child)};
}

Solution non_strict_2cc::GeneticAlgorithm::Selection(std::vector<Solution> population) {
  std::vector<Solution> solutions;
  std::random_device rd_;
  std::default_random_engine gen{rd_()};
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

std::vector<Solution> non_strict_2cc::GeneticAlgorithm::GenerateInitPopulation(unsigned int population_size) {
  auto generator = non_strict_2cc::NeighborhoodWithOneLocalSearch(num_threads_, factory_);
  auto solutions = generator.getAllSolutions(*graph_);
  unsigned best_distance = UINT_MAX;
  for (auto &it: solutions) {
    if (it.distance < best_distance) {
      best_distance = it.distance;
    }
  }
  std::vector<Solution> result;
  for (auto &it: solutions) {
    if (it.distance > best_distance) {
      result.emplace_back(it);
    } else {
      record_ = std::make_shared<Solution>(it);
    }
  }
  return result;
}

void non_strict_2cc::GeneticAlgorithm::OnIterationBegin(unsigned int iteration) {
  buffer_.clear();
}

void non_strict_2cc::GeneticAlgorithm::OnIterationEnd(unsigned int iteration) {
  population_.clear();
  population_.insert(population_.end(), buffer_.begin(), buffer_.end());
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
//    std::cout << "Update record on iteration " << iteration << std::endl;
  }

  if (num_iter_without_record_ == early_stop_num_) {
    stop_training_.store(true, std::memory_order_release);
  }
}

Solution non_strict_2cc::GeneticAlgorithm::Train(std::shared_ptr<IGraph> graph) {
  record_ = nullptr;
  graph_ = graph;
  buffer_ = GenerateInitPopulation(population_size_);
  population_.clear();
  for (auto &it: buffer_) {
    if (!IsIn(it, population_)) {
      population_.emplace_back(it);
    }
  }

  num_iter_without_record_ = 0;
  stop_training_.store(false, std::memory_order_release);

  std::vector<std::thread> threads(num_threads_);
  std::vector<std::vector<Solution>> local_buffers(num_threads_);
  barrier_ = std::make_unique<std::barrier<>>(num_threads_ + 1);

  for (unsigned i = 0; i < num_threads_; i++) {
    unsigned max_capacity = population_size_ / num_threads_;
    threads[i] = std::thread(
        &GeneticAlgorithm::WorkerLoop,
        this,
        std::ref(local_buffers[i]),
        max_capacity
    );
  }

  for (unsigned i = 0; i < iterations_; i++) {
    OnIterationBegin(i);

    barrier_->arrive_and_wait();
    barrier_->arrive_and_wait();

    for (auto &it: local_buffers) {
      buffer_.insert(buffer_.end(), it.begin(), it.end());
      it.clear();
    }

    OnIterationEnd(i);
    std::cout << "Iteration " << i << " done" << std::endl;
    if (stop_training_) {
      std::cout << "stop training" << std::endl;
      break;
    }
  }

  stop_training_ = true;
  barrier_->arrive_and_drop();
  std::cout << "Barrier done" << std::endl;
  for (auto &it: threads) {
    it.join();
  }

  return *record_;
}

IClustPtr non_strict_2cc::GeneticAlgorithm::CreateClusteringByBases(std::vector<std::set<unsigned int>> &bases) {
  auto child = factory_->CreateClustering(graph_->Size());
  for (unsigned i = 0; i < bases.size(); i++) {
    auto base = bases[i];
    for (auto &it: base) {
      if (i == 0) {
        child->SetupLabelForVertex(it, FIRST_CLUSTER);
      } else {
        child->SetupLabelForVertex(it, SECOND_CLUSTER);
      }
    }
  }
  return child;
}

non_strict_2cc::ToMergeCandidate non_strict_2cc::GeneticAlgorithm::FindCandidateToMerge(std::vector<std::set<unsigned int>> &bases) {
  ToMergeCandidate candidate;
  for (unsigned i = 0; i < bases.size(); i++) {
    for (unsigned j = i + 1; j < bases.size(); j++) {
      int local_record = 0;
      std::set<unsigned> set_i = bases[i];
      std::set<unsigned> set_j = bases[j];
      for (auto &it1: set_i) {
        for (auto &it2: set_j) {
          auto is_joined = graph_->IsJoined(it1, it2);
          if (is_joined) {
            local_record--;
          } else {
            local_record++;
          }
        }
      }
      if (local_record < candidate.record) {
        candidate.i = i;
        candidate.j = j;
        candidate.record = local_record;
      }
    }
  }
  return candidate;
}

void non_strict_2cc::GeneticAlgorithm::ThreadWorker(std::vector<Solution> &local_buffer,
                                                    const unsigned max_capacity,
                                                    const unsigned iteration) {
  while (local_buffer.size() < max_capacity) {
    auto x = Selection(population_);
    auto y = Selection(population_);
    auto pair = Crossover(x, y);
    for (const auto &it: pair) {
      x = Mutation(it, p_mutation_);
      if (local_buffer.size() < max_capacity) {
        local_buffer.emplace_back(x);
      }
    }
  }
  if (iteration % 5 == 0 && iteration > 0) {
    std::vector<Solution> tmp_buffer;
    for (auto &it: local_buffer) {
      auto x = it;
      for (unsigned j = 0; j < 5; j++) {
        x = ShakeUp(x, 1e-1);
      }
      tmp_buffer.emplace_back(x);
    }
    local_buffer.clear();
    local_buffer.insert(local_buffer.end(), tmp_buffer.begin(), tmp_buffer.end());
  }
}

void non_strict_2cc::GeneticAlgorithm::WorkerLoop(std::vector<Solution> &local_buffer, unsigned int max_capacity) {
  unsigned iteration = 0;
  while (true) {
    barrier_->arrive_and_wait();
    if (stop_training_.load(std::memory_order_acquire)) {
      barrier_->arrive_and_drop();
      break;
    }
    ThreadWorker(local_buffer, max_capacity, iteration);
    iteration++;
    barrier_->arrive_and_wait();
  }
}


