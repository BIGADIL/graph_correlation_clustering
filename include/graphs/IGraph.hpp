#pragma once

#include <memory>

/**
 * Base interface for graph.
 */
class IGraph {
 public:
  /**
   * Check if two vertices are joined.
   *
   * @param i first vertex.
   * @param j second vertex.
   * @return @code true, if vertices are joined.
   */
  virtual bool IsJoined(unsigned i, unsigned j) const = 0;
  virtual ~IGraph() = default;
  /**
   * Get size of graph.
   *
   * @return size of graph.
   */
  virtual unsigned Size() const = 0;
  /**
   * Generate json-string of graph.
   *
   * @return json-string of graph.
   */
  virtual std::string ToJson() const = 0;

  virtual bool operator==(IGraph &other) const = 0;
};

using IGraphPtr = std::shared_ptr<IGraph>;
