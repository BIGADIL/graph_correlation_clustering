#pragma once

#include <memory>

#include "../IClustering.hpp"

/**
 * Base interface.
 *
 * Factory creates new clustering.
 */
class IClusteringFactory {
 private:
  IClusteringFactory(const IClusteringFactory&) = delete;
  IClusteringFactory(const IClusteringFactory&&) = delete;
  IClustering& operator=(const IClusteringFactory&) = delete;
  IClustering& operator=(const IClusteringFactory&&) = delete;

 public:
  IClusteringFactory() = default;
  /**
   * Function creates new clustering vector.
   * @param size length of new clustering.
   * @return new clustering.
   */
  virtual IClustPtr CreateClustering(const unsigned size) const = 0;
};

using IClustFactoryPtr = std::shared_ptr<IClusteringFactory>;
