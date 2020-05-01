#include "../../../include/clustering/factories/BinaryClusteringFactory.hpp"
#include "../../../include/clustering/BinaryClusteringVector.hpp"

std::shared_ptr<IClustering> BinaryClusteringFactory::CreateClustering(const unsigned size) const {
  return std::shared_ptr<IClustering>(new BinaryClusteringVector(size));
}
