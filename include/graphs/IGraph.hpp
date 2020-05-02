#pragma once

class IGraph {
 public:
  virtual bool IsJoined(unsigned i, unsigned j) const = 0;
  virtual ~IGraph() = default;
  virtual unsigned Size() const = 0;
  virtual std::string ToJson() const = 0;
};
