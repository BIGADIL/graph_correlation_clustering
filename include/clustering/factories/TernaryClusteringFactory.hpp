#pragma once

#include "IClusteringFactory.hpp"

class TernaryClusteringFactory : public IClusteringFactory {
 public:
  IClustPtr CreateClustering(const unsigned size) const override;
};



