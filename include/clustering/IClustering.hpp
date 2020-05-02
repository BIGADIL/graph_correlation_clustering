#pragma once

#include "../graphs/IGraph.hpp"
#include "../common/CLusteringLabels.hpp"

class IClustering {
 public:
  virtual void SetupLabelForVertex(const unsigned vertex, const ClusterLabels label) = 0;
  virtual unsigned GetDistanceToGraph(const IGraph &graph) const = 0;
  virtual std::shared_ptr<IClustering> GetCopy() const = 0;
  virtual ClusterLabels GetLabel(const unsigned vertex) const = 0;
  virtual bool IsNonClustered(const unsigned vertex) const = 0;
  virtual bool IsSameClustered(const unsigned i, const unsigned j) const = 0;
  virtual unsigned GetNumNonClusteredVertices() const = 0;
  virtual unsigned GetNumVerticesByLabel(const unsigned label) const = 0;
  virtual std::string ToJson() const = 0;
};



