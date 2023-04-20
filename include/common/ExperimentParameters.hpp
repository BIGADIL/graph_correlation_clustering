#pragma once

#include <fstream>
#include <vector>

/**
 * Dot-class contains parameters for experiment.
 */
class ExperimentParameters {
 private:
  /**
   * Vector contains sizes of graphs which should be explored.
   */
  const std::vector<unsigned> graph_size_vector_;
  /**
   * Vector contains densities of investigated graphs.
   */
  const std::vector<double> density_vector_;
  /**
   * Vector contains names of algorithm which solves clustering problem.
   */
  const std::vector<std::string> algorithms_vector_;
  /**
   * For semi-supervised clustering this vector contains number of pre-clustered vertices.
   */
  const std::vector<double> parts_;
  /**
   * Number of threads which uses in solver.
   */
  const unsigned num_threads_;
  /**
   * Number of graphs for which series of tasks.
   */
  const unsigned num_graphs_;

 public:
  ExperimentParameters(std::vector<unsigned> graph_size_vector,
                       std::vector<double> density_vector,
                       std::vector<std::string> algorithms_vector,
                       std::vector<double> parts,
                       unsigned num_threads,
                       unsigned num_graphs);

  const std::vector<unsigned int> &GetGraphSize() const;
  const std::vector<double> &GetDensity() const;
  const std::vector<std::string> &GetAlgorithms() const;
  unsigned int GetNumThreads() const;
  unsigned int GetNumGraphs() const;
  const std::vector<double> &GetParts() const;
  static ExperimentParameters readFromConfig(const std::string &path);
};



