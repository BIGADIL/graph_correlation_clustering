#pragma once

#include "IClusteringFactory.hpp"

/**
 * Factory creates binary clustering.
 */
class BinaryClusteringFactory : public IClusteringFactory {
 public:
  [[nodiscard]] IClustPtr CreateClustering(unsigned size) const override;
};