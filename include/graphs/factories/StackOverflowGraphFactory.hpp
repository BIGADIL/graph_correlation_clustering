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
  std::uniform_int_distribution<> dis_;
  std::uniform_real_distribution<> noise_generator_{0.0, 1.0};
  double noise_;

  static bool HasCommonElement(const std::vector<std::string>& vec1, const std::vector<std::string>& vec2);

 public:
  explicit StackOverflowGraphFactory(const std::string &path, const double noise);
  IGraphPtr CreateGraph(unsigned size) override;
};
