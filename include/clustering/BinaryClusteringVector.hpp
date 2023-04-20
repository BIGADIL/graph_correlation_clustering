#pragma once

#include <vector>
#include <memory>

#include "IClustering.hpp"

/**
 * Binary clustering vector contains label (0 or 1) for each vertex.
 *
 * Vector may contain label -1 for non-clustered vertices.
 */
class BinaryClusteringVector : public IClustering {
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

 protected:
  BinaryClusteringVector() = delete;
  BinaryClusteringVector(const BinaryClusteringVector &) = default;
  BinaryClusteringVector(const BinaryClusteringVector &&) = delete;
  BinaryClusteringVector &operator=(const BinaryClusteringVector &) = default;
  BinaryClusteringVector &operator=(const BinaryClusteringVector &&) = delete;

  BinaryClusteringVector(const unsigned size);

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

  friend class BinaryClusteringFactory;
};
