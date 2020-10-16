#pragma once

#include <vector>
#include "IClustering.hpp"

class BFBinaryClusteringVector : public IClustering {
  /**
   * Vector of labels.
   */
  std::vector<int> labels_;

 protected:
  BFBinaryClusteringVector() = delete;
  BFBinaryClusteringVector(const BFBinaryClusteringVector &) = default;
  BFBinaryClusteringVector(const BFBinaryClusteringVector &&) = delete;
  BFBinaryClusteringVector &operator=(const BFBinaryClusteringVector &) = default;
  BFBinaryClusteringVector &operator=(const BFBinaryClusteringVector &&) = delete;

  BFBinaryClusteringVector Copy() const;

 public:
  BFBinaryClusteringVector(const unsigned size);
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



