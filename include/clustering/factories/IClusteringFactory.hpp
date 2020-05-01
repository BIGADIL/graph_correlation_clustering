#pragma once

#include <memory>
#include "../IClustering.hpp"
class IClusteringFactory {
 private:
  IClusteringFactory(const IClusteringFactory&) = delete;
  IClusteringFactory(const IClusteringFactory&&) = delete;
  IClustering& operator=(const IClusteringFactory&) = delete;
  IClustering& operator=(const IClusteringFactory&&) = delete;

 public:
  IClusteringFactory() = default;
  virtual std::shared_ptr<IClustering> CreateClustering(const unsigned size) const = 0;
};



