#pragma once

#include "IClusteringFactory.hpp"

/**
 * Factory creates triple clustering.
 */
class TripleClusteringFactory : public IClusteringFactory {
 public:
  [[nodiscard]] IClustPtr CreateClustering(unsigned size) const override;
};



