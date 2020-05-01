#pragma once

#include "../graphs/IGraph.hpp"
class IClustering {
 public:
  virtual void SetupLabelForVertex(const unsigned vertex, const unsigned label) = 0;
  virtual unsigned GetDistanceToGraph(const IGraph &graph) const = 0;
  virtual std::shared_ptr<IClustering> GetCopy() const = 0;
  virtual int GetLabel(const unsigned vertex) const = 0;
  virtual bool IsNonClustered(const unsigned vertex) const = 0;
  virtual bool IsSameClustered(const unsigned i, const unsigned j) const = 0;
  virtual unsigned GetNumNonClusteredVertices() const = 0;
  virtual unsigned GetNumVerticesByLabel(const unsigned label) const = 0;
};



