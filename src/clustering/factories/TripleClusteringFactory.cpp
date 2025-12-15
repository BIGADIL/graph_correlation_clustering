#include "../../../include/clustering/factories/TripleClusteringFactory.hpp"
#include "../../../include/clustering/TripleClusteringVector.hpp"

IClustPtr TripleClusteringFactory::CreateClustering(const unsigned int size) const {
    return std::make_shared<TripleClusteringVector>(size);
}
