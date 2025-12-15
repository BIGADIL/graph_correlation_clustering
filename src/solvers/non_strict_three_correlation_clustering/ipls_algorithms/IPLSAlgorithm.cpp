#include "../../../../include/solvers/non_strict_three_correlation_clustering/ipls_algorithms/IPLSAlgorithm.hpp"

#include <algorithm>
#include <climits>
#include <thread>

#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/LocalSearch.hpp"

non_strict_3cc::IPLSAlgorithm::IPLSAlgorithm(const unsigned iterations,
                                             const unsigned early_stop_num,
                                             IClustFactoryPtr factory,
                                             const unsigned population_size,
                                             const unsigned tournament_size,
                                             const double p_perturbation,
                                             const unsigned num_threads)
    : iterations_(iterations),
      early_stop_num_(early_stop_num),
      factory_(std::move(factory)),
      population_size_(population_size),
      tournament_size_(tournament_size),
      p_perturbation_(p_perturbation),
      num_threads_(num_threads) {}

Solution non_strict_3cc::IPLSAlgorithm::LocalSearch(
    const Solution &solution) const {
  const auto copy = solution.clustering->GetCopy();
  auto lo = LocalSearch::ComputeLocalOptimum(*graph_, copy);
  return {lo->GetDistanceToGraph(*graph_), lo};
}

Solution non_strict_3cc::IPLSAlgorithm::Perturbation(
    const Solution &solution, const double p_perturbation) const {
  auto copy = solution.clustering->GetCopy();
  std::random_device rd_;
  std::default_random_engine gen{rd_()};
  std::uniform_real_distribution<> dis;
  for (unsigned i = 0; i < graph_->Size(); i++) {
    auto p = dis(gen);
    if (p > p_perturbation) {
      continue;
    }
    auto imp = LocalSearch::ComputeLocalImprovement(*graph_, copy, i);
    const auto label_i = copy->GetLabel(i);
    ClusterLabels label = NON_CLUSTERED;
    int best_imp = INT_MIN;
    for (unsigned j = 0; j < imp.size(); j++) {
      if (static_cast<ClusterLabels>(j) == label_i) continue;
      if (imp[j] > best_imp) {
        best_imp = imp[j];
        label = static_cast<ClusterLabels>(j);
      }
    }
    copy->SetupLabelForVertex(i, label);
  }
  return {copy->GetDistanceToGraph(*graph_), copy};
}

Solution non_strict_3cc::IPLSAlgorithm::Selection(
    std::vector<Solution> population) const {
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

void non_strict_3cc::IPLSAlgorithm::GenerateInitPopulation(
    unsigned int population_size) {
  std::vector<std::vector<Solution> > local_buffers(num_threads_);
  std::vector<std::thread> threads(num_threads_);
  for (unsigned i = 0; i < num_threads_; ++i) {
    threads[i] = std::thread(&IPLSAlgorithm::InitPopulationWorker, this,
                             std::ref(local_buffers[i]), i, population_size);
  }
  for (auto &item : threads) {
    item.join();
  }
  population_.clear();
  for (auto &it : local_buffers) {
    population_.insert(population_.end(), it.begin(), it.end());
  }
}

void non_strict_3cc::IPLSAlgorithm::OnIterationBegin() { buffer_.clear(); }

void non_strict_3cc::IPLSAlgorithm::OnIterationEnd() {
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
  }

  if (num_iter_without_record_ == early_stop_num_) {
    stop_training_.store(true, std::memory_order_release);
  }
}

Solution non_strict_3cc::IPLSAlgorithm::Train(
    const std::shared_ptr<IGraph> &graph) {
  record_ = nullptr;
  graph_ = graph;
  GenerateInitPopulation(population_size_);

  num_iter_without_record_ = 0;
  stop_training_.store(false, std::memory_order_release);

  std::vector<std::thread> threads(num_threads_);
  std::vector<std::vector<Solution> > local_buffers(num_threads_);
  std::vector<std::vector<Solution> > local_optimum(num_threads_);
  barrier_ = std::make_unique<std::barrier<> >(num_threads_ + 1);

  for (unsigned i = 0; i < num_threads_; i++) {
    threads[i] = std::thread(&IPLSAlgorithm::TrainWorkerLoop, this,
                             std::ref(local_buffers[i]), i, population_size_,
                             std::ref(local_optimum[i]));
  }

  for (unsigned i = 0; i < iterations_; i++) {
    OnIterationBegin();

    barrier_->arrive_and_wait();
    barrier_->arrive_and_wait();

    population_.clear();
    for (auto &it : local_buffers) {
      population_.insert(population_.end(), it.begin(), it.end());
      it.clear();
    }
    buffer_.clear();
    for (auto &it : local_optimum) {
      buffer_.insert(buffer_.end(), it.begin(), it.end());
      it.clear();
    }

    OnIterationEnd();
    if (stop_training_) {
      break;
    }
  }

  stop_training_ = true;
  barrier_->arrive_and_drop();
  for (auto &it : threads) {
    it.join();
  }

  return *record_;
}

void non_strict_3cc::IPLSAlgorithm::TrainWorker(
    std::vector<Solution> &local_buffer, const unsigned thread_id,
    const unsigned max_capacity, std::vector<Solution> &local_optimum) const {
  Solution solution{UINT_MAX, nullptr};
  for (unsigned i = thread_id; i < max_capacity; i += num_threads_) {
    auto x = Selection(population_);
    x = LocalSearch(x);
    local_buffer.emplace_back(x);
    if (x.distance < solution.distance) {
      solution = x;
    }
  }
  std::vector<Solution> tmp_buffer;
  for (const auto &it : local_buffer) {
    auto x = it;
    x = Perturbation(x, p_perturbation_);
    tmp_buffer.emplace_back(x);
    if (x.distance < solution.distance) {
      solution = x;
    }
  }
  local_buffer.clear();
  local_buffer.insert(local_buffer.end(), tmp_buffer.begin(), tmp_buffer.end());
  local_optimum.emplace_back(solution);
}

void non_strict_3cc::IPLSAlgorithm::TrainWorkerLoop(
    std::vector<Solution> &local_buffer, const unsigned thread_id,
    const unsigned int max_capacity,
    std::vector<Solution> &local_optimum) const {
  while (true) {
    barrier_->arrive_and_wait();
    if (stop_training_.load(std::memory_order_acquire)) {
      barrier_->arrive_and_drop();
      break;
    }
    TrainWorker(local_buffer, thread_id, max_capacity, local_optimum);
    barrier_->arrive_and_wait();
  }
}

void non_strict_3cc::IPLSAlgorithm::InitPopulationWorker(
    std::vector<Solution> &local_buffer, const unsigned int thread_id,
    const unsigned int population_size) const {
  std::random_device rd_;
  std::default_random_engine gen{rd_()};
  std::uniform_int_distribution<> dis(0, 2);

  for (unsigned i = thread_id; i < population_size; i += num_threads_) {
    const auto clustering = factory_->CreateClustering(graph_->Size());
    for (unsigned j = 0; j < graph_->Size(); ++j) {
      if (const auto rnd = dis(gen); rnd == 0) {
        clustering->SetupLabelForVertex(j, FIRST_CLUSTER);
      } else if (rnd == 1) {
        clustering->SetupLabelForVertex(j, SECOND_CLUSTER);
      } else {
        clustering->SetupLabelForVertex(j, THIRD_CLUSTER);
      }
    }
    auto sol = Solution(clustering->GetDistanceToGraph(*graph_), clustering);
    local_buffer.emplace_back(sol);
  }
}
