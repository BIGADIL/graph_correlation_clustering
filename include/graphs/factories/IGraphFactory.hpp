#pragma once

#include <memory>
#include "../IGraph.hpp"

class IGraphFactory {
 public:
  virtual std::shared_ptr<IGraph> CreateGraph(const unsigned size) = 0;
};



