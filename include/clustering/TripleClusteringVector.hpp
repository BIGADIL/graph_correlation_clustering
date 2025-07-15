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

 public:
  TripleClusteringVector() = delete;
  TripleClusteringVector(const TripleClusteringVector &) = default;
  TripleClusteringVector(const TripleClusteringVector &&) = delete;
  TripleClusteringVector &operator=(const TripleClusteringVector &) = default;
  TripleClusteringVector &operator=(const TripleClusteringVector &&) = delete;

  explicit TripleClusteringVector(unsigned size);

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

  friend class TripleClusteringFactory;
};



