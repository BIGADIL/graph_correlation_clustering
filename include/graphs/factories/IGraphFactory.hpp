#pragma once

#include <memory>
#include "../IGraph.hpp"

/**
 * Base factory for graphs.
 */
class IGraphFactory {
 public:
  /**
   * Create new graph.
   * @param size size of graph.
   * @return new graph.
   */
  virtual IGraphPtr CreateGraph(const unsigned size) = 0;
};
