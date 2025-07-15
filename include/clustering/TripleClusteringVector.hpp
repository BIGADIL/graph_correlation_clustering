#pragma once

#include <vector>
#include "IClustering.hpp"

class TripleClusteringVector : public IClustering {
 protected:
  /**
   * Vector of labels.
   */
  std::vector<ClusterLabels> labels_;
  /**
   * Number of non-clustered vertices.
   */
  unsigned num_non_clustered_vertices_;
  /**
   * Number of vertices from first cluster.
   */
  unsigned num_vertices_in_first_cluster_;
  /**
   * Number of vertices from second cluster.
   */
  unsigned num_vertices_in_second_cluster_;
  /**
   * Number of vertices from third cluster.
   */
  unsigned num_vertices_in_third_cluster_;

 protected:
  TripleClusteringVector() = delete;
  TripleClusteringVector(const TripleClusteringVector &) = default;
  TripleClusteringVector(const TripleClusteringVector &&) = delete;
  TripleClusteringVector &operator=(const TripleClusteringVector &) = default;
  TripleClusteringVector &operator=(const TripleClusteringVector &&) = delete;

  TripleClusteringVector(const unsigned size);

 public:
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
  unsigned Size() const override;

  friend class TripleClusteringFactory;
};



