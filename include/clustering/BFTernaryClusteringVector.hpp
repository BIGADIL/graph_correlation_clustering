#pragma once

#include <vector>
#include "IClustering.hpp"
class BFTernaryClusteringVector : public IClustering {
  /**
   * Vector of labels.
   */
  std::vector<int> labels_;

 protected:
  BFTernaryClusteringVector() = delete;
  BFTernaryClusteringVector(const BFTernaryClusteringVector &) = default;
  BFTernaryClusteringVector(const BFTernaryClusteringVector &&) = delete;
  BFTernaryClusteringVector &operator=(const BFTernaryClusteringVector &) = default;
  BFTernaryClusteringVector &operator=(const BFTernaryClusteringVector &&) = delete;

  BFTernaryClusteringVector Copy() const;

 public:
  BFTernaryClusteringVector(const unsigned size);
  void SetupLabelForVertex(const unsigned vertex,
                           const ClusterLabels label) override;
  unsigned GetDistanceToGraph(const IGraph &graph) const override;
  IClustPtr GetCopy() const override;
  ClusterLabels GetLabel(const unsigned vertex) const override;
  bool IsNonClustered(const unsigned vertex) const override;
  bool IsSameClustered(const unsigned i,
                       const unsigned j) const override;
  unsigned GetNumNonClusteredVertices() const override;
  unsigned int GetNumVerticesByLabel(const ClusterLabels label) const override;
  std::string ToJson() const override;

  IClustPtr getNextClustering();
};



