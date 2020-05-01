#pragma once

#include "IClusteringFactory.hpp"
class BinaryClusteringFactory : public IClusteringFactory {
 public:
  std::shared_ptr<IClustering> CreateClustering(const unsigned size) const override;
};



