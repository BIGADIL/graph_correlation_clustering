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
  [[nodiscard]] virtual bool IsJoined(unsigned i, unsigned j) const = 0;
  virtual ~IGraph() = default;
  /**
   * Get size of graph.
   *
   * @return size of graph.
   */
  [[nodiscard]] virtual unsigned Size() const = 0;
  /**
   * Generate json-string of graph.
   *
   * @return json-string of graph.
   */
  [[nodiscard]] virtual std::string ToJson() const = 0;

  virtual bool operator==(IGraph &other) const = 0;
};

using IGraphPtr = std::shared_ptr<IGraph>;
