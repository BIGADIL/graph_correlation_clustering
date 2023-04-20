#pragma once

#include "IClusteringFactory.hpp"

/**
 * Factory creates binary clustering.
 */
class BinaryClusteringFactory : public IClusteringFactory {
 public:
  IClustPtr CreateClustering(const unsigned size) const override;
};