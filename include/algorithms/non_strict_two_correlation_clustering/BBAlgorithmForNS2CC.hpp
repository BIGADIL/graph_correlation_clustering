#pragma once

#include "../../clustering/factories/IClusteringFactory.hpp"
#include "../../clustering/BBBinaryClusteringVector.hpp"
#include "../../clustering/IClustering.hpp"

namespace ns2cc {

  /**
   * Base branch and bound algorithm for NS2CC.
   *
   * It works only with ::BBBinaryClusteringVector.
   */
  class BBAlgorithmForNS2CC {
   private:
    /**
     * The source graph to be clustered.
     */
    IGraphPtr graph_;
    /**
     * Record value.
     * @see ::GetBestClustering
     */
    unsigned record_{0};
    /**
     * Best clustering.
     * @see ::GetBestClustering
     */
    IClustPtr best_clustering_ = nullptr;

   private:
    /**
     * Branch out the range of feasible solutions.
     * @param clustering current vector of labels.
     */
    void Branch(BBBinaryClusteringVector &clustering);

   public:
    explicit BBAlgorithmForNS2CC() = default;
    /**
     * Get optimal solution for source graph.
     * @param graph source graph.
     * @param initial_clustering initial clustering.
     * @return optimal solution for source graph.
     */
    std::shared_ptr<IClustering> GetBestClustering(const IGraphPtr &graph,
                                                   const IClustPtr &initial_clustering);
  };
}


