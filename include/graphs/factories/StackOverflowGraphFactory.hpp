#pragma once

#include <unordered_map>
#include <vector>
#include <random>
#include "IGraphFactory.hpp"
/**
 * Factory creates new random graph by Erdos-Renyi model.
 * https://en.wikipedia.org/wiki/Erdős–Rényi_model
 */
class StackOverflowGraphFactory : IGraphFactory {
 private:
  std::unordered_map<std::string, std::vector<std::string>> data_;
  std::vector<std::string> keys_;
  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::string distribution_;

  static double Chance(const std::vector<std::string>& vec1, const std::vector<std::string>& vec2);

 public:
  explicit StackOverflowGraphFactory(const std::string &path, std::string distribution_);
  IGraphPtr CreateGraph(unsigned size) override;
};
