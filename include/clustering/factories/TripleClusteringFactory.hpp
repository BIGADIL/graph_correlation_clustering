#pragma once

#include "IClusteringFactory.hpp"

/**
 * Factory creates triple clustering.
 */
class TripleClusteringFactory : public IClusteringFactory {
 public:
  IClustPtr CreateClustering(const unsigned size) const override;
};



