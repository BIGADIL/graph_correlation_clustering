#pragma once

#include <random>
#include <unordered_map>
#include <vector>

#include "IGraphFactory.hpp"
/**
 * Factory creates graphs by set of tags.
 * Example: https://www.kaggle.com/datasets/stackoverflow/stacksample/
 */
class TagsGraphFactory final : IGraphFactory {
  std::unordered_map<std::string, std::vector<std::string> > data_;
  std::vector<std::string> keys_;
  std::random_device rd_;
  std::default_random_engine gen_{rd_()};
  std::string distribution_;

  double Chance(const std::vector<std::string> &vec1,
                const std::vector<std::string> &vec2) const;

 public:
  explicit TagsGraphFactory(const std::string &path, std::string distribution_);

  IGraphPtr CreateGraph(unsigned size) override;
};
