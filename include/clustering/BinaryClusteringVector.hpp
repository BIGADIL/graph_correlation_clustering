#pragma once

#include <vector>
#include <memory>

#include "IClustering.hpp"
#include "../common/Constants.hpp"

class BinaryClusteringVector : public IClustering {

 protected:
  std::vector<ClusterLabels> labels_;
  unsigned num_non_clustered_vertices_;
  unsigned num_vertices_in_first_cluster_;
  unsigned num_vertices_in_second_cluster_;

 protected:
  BinaryClusteringVector() = delete;
  BinaryClusteringVector(const BinaryClusteringVector &) = default;
  BinaryClusteringVector(const BinaryClusteringVector &&) = delete;
  BinaryClusteringVector &operator=(const BinaryClusteringVector &) = delete;
  BinaryClusteringVector &operator=(const BinaryClusteringVector &&) = delete;

  BinaryClusteringVector(const unsigned size);

 public:
  void SetupLabelForVertex(const unsigned vertex, const ClusterLabels label) override;
  unsigned GetDistanceToGraph(const IGraph &graph) const override;
  std::shared_ptr<IClustering> GetCopy() const override;
  ClusterLabels GetLabel(const unsigned vertex) const override;
  bool IsNonClustered(const unsigned vertex) const override;
  bool IsSameClustered(const unsigned i, const unsigned j) const override;
  unsigned GetNumNonClusteredVertices() const override;
  unsigned int GetNumVerticesByLabel(const unsigned label) const override;

  friend class BinaryClusteringFactory;
};



