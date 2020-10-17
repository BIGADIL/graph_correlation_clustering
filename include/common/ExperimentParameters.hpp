#pragma once

#include <fstream>
#include <vector>

class ExperimentParameters {

 private:
  std::vector<unsigned> graph_size_vector_;
  std::vector<double> density_vector_;
  std::vector<std::string> algorithms_vector_;
  unsigned num_threads_;
  unsigned num_graphs_;

 public:
  ExperimentParameters(std::vector<unsigned> graph_size_vector,
                       std::vector<double> density_vector,
                       std::vector<std::string> algorithms_vector,
                       unsigned num_threads,
                       unsigned num_graphs);

  const std::vector<unsigned int> &GetGraphSize() const;
  const std::vector<double> &GetDensity() const;
  const std::vector<std::string> &GetAlgorithms() const;
  unsigned int GetNumThreads() const;
  unsigned int GetNumGraphs() const;
  static ExperimentParameters readFromConfig(const std::string &path);
};



