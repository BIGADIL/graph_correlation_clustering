#pragma once

#include "IClusteringFactory.hpp"

/**
 * Factory creates binary clustering.
 */
class BinaryClusteringFactory final : public IClusteringFactory {
 public:
  [[nodiscard]] IClustPtr CreateClustering(unsigned size) const override;
};
