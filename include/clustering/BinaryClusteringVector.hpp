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
  explicit BinaryClusteringVector(unsigned size);

 public:
  BinaryClusteringVector() = delete;
  BinaryClusteringVector(const BinaryClusteringVector &) = default;
  BinaryClusteringVector(const BinaryClusteringVector &&) = delete;
  BinaryClusteringVector &operator=(const BinaryClusteringVector &) = default;
  BinaryClusteringVector &operator=(const BinaryClusteringVector &&) = delete;
  void SetupLabelForVertex(unsigned vertex,
                           ClusterLabels label) override;
  [[nodiscard]] unsigned GetDistanceToGraph(const IGraph &graph) const override;
  [[nodiscard]] IClustPtr GetCopy() const override;
  [[nodiscard]] ClusterLabels GetLabel(unsigned vertex) const override;
  [[nodiscard]] bool IsNonClustered(unsigned vertex) const override;
  [[nodiscard]] bool IsSameClustered(unsigned i,
                                     unsigned j) const override;
  [[nodiscard]] unsigned GetNumNonClusteredVertices() const override;
  [[nodiscard]] unsigned int GetNumVerticesByLabel(ClusterLabels label) const override;
  [[nodiscard]] std::string ToJson() const override;
  [[nodiscard]] unsigned Size() const override;

  friend class BinaryClusteringFactory;
};
