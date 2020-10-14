#pragma once

#include <memory>

#include "../IClustering.hpp"

/**
 * Base interface.
 *
 * Factory could create new clustering.
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
   * Create new clustering.
   * @param size size of new clustering.
   * @return new clustering.
   */
  virtual IClustPtr CreateClustering(const unsigned size) const = 0;
};

using IClustFactoryPtr = std::shared_ptr<IClusteringFactory>;
