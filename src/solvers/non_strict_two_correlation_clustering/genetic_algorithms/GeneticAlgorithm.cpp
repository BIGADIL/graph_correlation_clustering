
#include <algorithm>
#include <iostream>
#include <climits>
#include <thread>

#include "../../../../include/solvers/non_strict_two_correlation_clustering/genetic_algorithms/GeneticAlgorithm.hpp"
#include "../../../../include/solvers/non_strict_two_correlation_clustering/common_functions/LocalSearch.hpp"
#include "../../../../include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithOneLocalSearch.hpp"

non_strict_2cc::GeneticAlgorithm::GeneticAlgorithm(unsigned iterations,
                                                   unsigned early_stop_num,
                                                   IClustFactoryPtr factory,
                                                   unsigned population_size,
                                                   unsigned tournament_size,
                                                   double p_mutation,
                                                   unsigned num_threads) :
    num_threads_(num_threads),
    iterations_(iterations),
    early_stop_num_(early_stop_num),
    factory_(std::move(factory)),
    population_size_(population_size),
    tournament_size_(tournament_size),
    p_mutation_(p_mutation) {
}

Solution non_strict_2cc::GeneticAlgorithm::Mutation(const Solution &solution, double p_mutation) {
  auto copy = solution.clustering->GetCopy();
  auto lo = LocalSearch::ComputeLocalOptimum(*graph_, copy);
  return {lo->GetDistanceToGraph(*graph_), lo};
}

Solution non_strict_2cc::GeneticAlgorithm::Perturbation(const Solution &solution, double p_shake) {
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
  std::vector<std::vector<Solution>> local_buffers(num_threads_);
  std::vector<std::thread> threads(num_threads_);
  for (unsigned i = 0; i < num_threads_; ++i) {
    threads[i] = std::thread(
        &GeneticAlgorithm::InitPopulationWorker,
        this,
        std::ref(local_buffers[i]),
        i,
        population_size);
  }
  for (auto &item: threads) {
    item.join();
  }
  std::vector<Solution> result;
  for (auto &it: local_buffers) {
    result.insert(result.end(), it.begin(), it.end());
  }
  return result;
}

void non_strict_2cc::GeneticAlgorithm::OnIterationBegin(unsigned int iteration) {
  buffer_.clear();
}

void non_strict_2cc::GeneticAlgorithm::OnIterationEnd(unsigned int iteration) {
  Solution best = buffer_[0];
  for (unsigned i = 1; i < buffer_.size(); i++) {
    if (buffer_[i].distance < best.distance) {
      best = buffer_[i];
    }
  }

  if (record_ != nullptr && best.distance >= record_->distance) {
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
  population_ = GenerateInitPopulation(population_size_);

  num_iter_without_record_ = 0;
  stop_training_.store(false, std::memory_order_release);

  std::vector<std::thread> threads(num_threads_);
  std::vector<std::vector<Solution>> local_buffers(num_threads_);
  std::vector<std::vector<Solution>> local_optimum(num_threads_);
  barrier_ = std::make_unique<std::barrier<>>(num_threads_ + 1);

  for (unsigned i = 0; i < num_threads_; i++) {
    threads[i] = std::thread(
        &GeneticAlgorithm::WorkerLoop,
        this,
        std::ref(local_buffers[i]),
        i,
        population_size_,
        std::ref(local_optimum[i])
    );
  }

  for (unsigned i = 0; i < iterations_; i++) {
    OnIterationBegin(i);

    barrier_->arrive_and_wait();
    barrier_->arrive_and_wait();

    population_.clear();
    for (auto &it: local_buffers) {
      population_.insert(population_.end(), it.begin(), it.end());
      it.clear();
    }
    buffer_.clear();
    for (auto &it: local_optimum) {
      buffer_.insert(buffer_.end(), it.begin(), it.end());
      it.clear();
    }

    OnIterationEnd(i);
//    std::cout << "Iteration " << i << " done" << std::endl;
    if (stop_training_) {
//      std::cout << "stop training" << std::endl;
      break;
    }
  }

  stop_training_ = true;
  barrier_->arrive_and_drop();
//  std::cout << "Barrier done" << std::endl;
  for (auto &it: threads) {
    it.join();
  }

  return *record_;
}

void non_strict_2cc::GeneticAlgorithm::ThreadWorker(std::vector<Solution> &local_buffer,
                                                    unsigned thread_id,
                                                    const unsigned max_capacity,
                                                    std::vector<Solution> &local_optimum,
                                                    const unsigned iteration) {
  Solution solution{UINT_MAX, nullptr};
  for (unsigned i = thread_id; i < max_capacity; i += num_threads_) {
    auto x = Selection(population_);
    x = Mutation(x, p_mutation_);
    local_buffer.emplace_back(x);
    if (x.distance < solution.distance) {
      solution = x;
    }
  }
  std::vector<Solution> tmp_buffer;
  for (auto &it: local_buffer) {
    auto x = it;
    x = Perturbation(x, p_mutation_);
    tmp_buffer.emplace_back(x);
    if (x.distance < solution.distance) {
      solution = x;
    }
  }
  local_buffer.clear();
  local_buffer.insert(local_buffer.end(), tmp_buffer.begin(), tmp_buffer.end());
  local_optimum.emplace_back(solution);
}

void non_strict_2cc::GeneticAlgorithm::WorkerLoop(std::vector<Solution> &local_buffer,
                                                  unsigned thread_id,
                                                  unsigned int max_capacity,
                                                  std::vector<Solution> &local_optimum) {
  unsigned iteration = 0;
  while (true) {
    barrier_->arrive_and_wait();
    if (stop_training_.load(std::memory_order_acquire)) {
      barrier_->arrive_and_drop();
      break;
    }
    ThreadWorker(local_buffer, thread_id, max_capacity, local_optimum, iteration);
    iteration++;
    barrier_->arrive_and_wait();
  }
}

void non_strict_2cc::GeneticAlgorithm::InitPopulationWorker(std::vector<Solution> &local_buffer,
                                                            unsigned int thread_id,
                                                            unsigned int population_size) {
  std::random_device rd_;
  std::default_random_engine gen{rd_()};
  std::uniform_int_distribution<> dis(0, 1);

  for (unsigned i = thread_id; i < population_size; i += num_threads_) {
    auto clustering = factory_->CreateClustering(graph_->Size());
    for (unsigned j = 0; j < graph_->Size(); ++j) {
      auto rnd = dis(gen);
      if (rnd == 0) {
        clustering->SetupLabelForVertex(j, FIRST_CLUSTER);
      } else {
        clustering->SetupLabelForVertex(j, SECOND_CLUSTER);
      }
    }
    auto sol = Solution(clustering->GetDistanceToGraph(*graph_), clustering);
    local_buffer.emplace_back(sol);
  }
}


