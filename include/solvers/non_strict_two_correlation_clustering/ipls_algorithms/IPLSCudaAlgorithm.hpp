#pragma once

#include <memory>

#include "../../../clustering/IClustering.hpp"
#include "../../../clustering/factories/IClusteringFactory.hpp"
#include "../../../graphs/IGraph.hpp"

namespace non_strict_2cc {
/**
 * CUDA implementation of IPLSAlgorithm (iterated population local search)
 * for NS2CC.
 *
 * Semantics match IPLSAlgorithm: on every iteration each of the
 * `population_size` slots selects a parent by tournament, runs the greedy
 * local search of Coleman, Saunderson and Wirth to a local optimum, and then
 * perturbs it. The best solution seen over local optima and perturbed
 * solutions is the record; training stops after `iterations` iterations or
 * after `early_stop_num` iterations without a new record.
 *
 * The whole population lives in GPU memory. One CUDA block handles one
 * individual; threads of the block split the vertices between them, so both
 * the population and the inner O(n) steps of the local search run in
 * parallel. Only the per-individual distances are copied to the host every
 * iteration.
 *
 * The header is plain C++; the implementation is compiled by nvcc.
 */
class IPLSCudaAlgorithm {
  unsigned iterations_;
  unsigned early_stop_num_;
  IClustFactoryPtr factory_;
  unsigned population_size_;
  unsigned tournament_size_;
  double p_perturbation_;

 public:
  IPLSCudaAlgorithm() = delete;

  IPLSCudaAlgorithm(const IPLSCudaAlgorithm &) = delete;

  IPLSCudaAlgorithm(IPLSCudaAlgorithm &&) = delete;

  IPLSCudaAlgorithm &operator=(const IPLSCudaAlgorithm &) = delete;

  IPLSCudaAlgorithm &operator=(IPLSCudaAlgorithm &&) = delete;

  /**
   * @param iterations maximum number of iterations.
   * @param early_stop_num stop after this many iterations without a new
   * record.
   * @param factory factory used to build the returned clustering.
   * @param population_size number of individuals (CUDA blocks).
   * @param tournament_size number of candidates in tournament selection.
   * @param p_perturbation probability to flip each vertex in perturbation.
   */
  IPLSCudaAlgorithm(unsigned iterations, unsigned early_stop_num,
                    IClustFactoryPtr factory, unsigned population_size,
                    unsigned tournament_size, double p_perturbation);

  /**
   * Run the algorithm for the graph.
   *
   * @param graph source graph.
   * @return best found solution.
   */
  Solution Train(const std::shared_ptr<IGraph> &graph);

  /**
   * Compute the local optimum of a single clustering on the GPU.
   *
   * Produces exactly the same result as LocalSearch::ComputeLocalOptimum
   * (same greedy rule, same tie-breaking); intended for validation and for
   * one-off local searches on big graphs.
   *
   * @param graph source graph.
   * @param cur_clustering init clustering; every vertex must be labeled.
   * @return local optimal clustering.
   */
  static IClustPtr ComputeLocalOptimum(const IGraph &graph,
                                       const IClustPtr &cur_clustering);

  /**
   * @return @code true, if at least one CUDA device is usable.
   */
  static bool IsAvailable();
};
}  // namespace non_strict_2cc
