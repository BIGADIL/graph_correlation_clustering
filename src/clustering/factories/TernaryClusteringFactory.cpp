#include "../../../include/clustering/factories/TernaryClusteringFactory.hpp"
#include "../../../include/clustering/TernaryClusteringVector.hpp"

IClustPtr TernaryClusteringFactory::CreateClustering(const unsigned int size) const {
  return IClustPtr(new TernaryClusteringVector(size));
}
