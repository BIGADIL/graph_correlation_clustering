#pragma once

#include <fstream>
#include <vector>

class ExperimentParameters {

 private:
  std::vector<unsigned> graph_size_vector_;
  std::vector<double> density_vector_;
  std::vector<std::string> algorithms_vector_;

 public:
  ExperimentParameters(std::vector<unsigned> graph_size_vector,
                       std::vector<double> density_vector,
                       std::vector<std::string> algorithms_vector);

  [[nodiscard]] const std::vector<unsigned int> &GetGraphSize() const {
    return graph_size_vector_;
  }
  [[nodiscard]] const std::vector<double> &GetDensity() const {
    return density_vector_;
  }
  [[nodiscard]] const std::vector<std::string> &GetAlgorithms() const {
    return algorithms_vector_;
  }

  static ExperimentParameters readFromConfig(const std::string &path);
};



