#pragma once

#include <random>
#include <atomic>
#include <barrier>

#include "../../../clustering/factories/TripleClusteringFactory.hpp"

namespace non_strict_3cc {
    class IPLSAlgorithm {
        std::shared_ptr<Solution> record_ = nullptr;
        unsigned num_iter_without_record_ = 0;
        std::shared_ptr<IGraph> graph_ = nullptr;
        std::vector<Solution> population_;
        std::vector<Solution> buffer_;
        std::atomic<bool> stop_training_ = false;
        std::unique_ptr<std::barrier<> > barrier_;

        unsigned iterations_;
        unsigned early_stop_num_;
        IClustFactoryPtr factory_;
        unsigned population_size_;
        unsigned tournament_size_;
        double p_perturbation_;
        unsigned num_threads_;

        [[nodiscard]] Solution LocalSearch(const Solution &solution) const;

        [[nodiscard]] Solution Selection(std::vector<Solution> population) const;

        [[nodiscard]] Solution Perturbation(const Solution &solution, double p_perturbation) const;

        void GenerateInitPopulation(unsigned population_size);

        void OnIterationBegin();

        void OnIterationEnd();


        void TrainWorkerLoop(std::vector<Solution> &local_buffer,
                             unsigned thread_id,
                             unsigned max_capacity,
                             std::vector<Solution> &local_optimum) const;

        void TrainWorker(std::vector<Solution> &local_buffer,
                         unsigned thread_id,
                         unsigned max_capacity,
                         std::vector<Solution> &local_optimum) const;

        void InitPopulationWorker(std::vector<Solution> &local_buffer,
                                  unsigned thread_id,
                                  unsigned population_size) const;

    public:
        IPLSAlgorithm() = delete;

        IPLSAlgorithm(const IPLSAlgorithm &) = delete;

        IPLSAlgorithm(const IPLSAlgorithm &&) = delete;

        IPLSAlgorithm &operator=(const IPLSAlgorithm &) = delete;

        IPLSAlgorithm &operator=(const IPLSAlgorithm &&) = delete;

        IPLSAlgorithm(unsigned iterations,
                      unsigned early_stop_num,
                      IClustFactoryPtr factory,
                      unsigned population_size,
                      unsigned tournament_size,
                      double p_perturbation,
                      unsigned num_threads);

        Solution Train(const std::shared_ptr<IGraph> &graph);
    };
}
