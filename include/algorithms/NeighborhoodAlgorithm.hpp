#pragma once

#include <memory>
#include <mutex>

#include "../clustering/IClustering.hpp"
#include "../clustering/factories/IClusteringFactory.hpp"
#include "../common/Constants.hpp"
#include "NeighborSplitter.hpp"
#include "LocalSearchAlgorithm.hpp"

class NeighborhoodAlgorithm {
 private:
  unsigned num_threads_;
  std::shared_ptr<IClusteringFactory> clustering_factory_;
  NeighborSplitter neighbor_splitter_;
  LocalSearchAlgorithm local_search_algorithm_;

 private:
  NeighborhoodAlgorithm() = delete;
  NeighborhoodAlgorithm(const NeighborhoodAlgorithm &&) = delete;
  NeighborhoodAlgorithm &operator=(const NeighborhoodAlgorithm &) = delete;
  NeighborhoodAlgorithm &operator=(const NeighborhoodAlgorithm &&) = delete;

  void BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
                                              const unsigned threadId,
                                              IClusteringPointer &local_best_neighborhood_clustering) const;

 public:
  NeighborhoodAlgorithm(const unsigned num_threads,
                        const std::shared_ptr<IClusteringFactory> &clustering_factory);
  IClusteringPointer getBestNeighborhoodClustering(const IGraph &graph) const;
};