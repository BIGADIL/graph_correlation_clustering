#include "../../../include/clustering/factories/BinaryClusteringFactory.hpp"
#include "../../../include/clustering/BinaryClusteringVector.hpp"

IClustPtr BinaryClusteringFactory::CreateClustering(const unsigned size) const {
  return IClustPtr(new BinaryClusteringVector(size));
}
