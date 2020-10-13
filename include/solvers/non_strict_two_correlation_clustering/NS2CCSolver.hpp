#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <sstream>

#include "../../graphs/IGraph.hpp"
#include "../../clustering/IClustering.hpp"
#include "../../clustering/factories/IClusteringFactory.hpp"

struct ClusteringInfo {
  std::string name;
  std::shared_ptr<IClustering> clustering;
  unsigned objective_function_value;
  std::chrono::seconds computation_time;

  ClusteringInfo(std::string name,
                 std::shared_ptr<IClustering> clustering,
                 unsigned int objective_function_value,
                 std::chrono::seconds computation_time)
      : name(std::move(name)),
        clustering(std::move(clustering)),
        objective_function_value(objective_function_value),
        computation_time(computation_time) {}

  std::string ToJson() const {
    std::stringstream ss;
    ss << "\"" << name << "\": {\n";
    ss << clustering->ToJson() << "," << std::endl;
    ss << "\"objective function value\": " << objective_function_value << "," << std::endl;
    ss << "\"computation time seconds\": " << computation_time.count() << "}";
    return ss.str();
  }
};

class NS2CCSolver {
 private:
  const std::vector<std::string> allowed_algorithms{"NNLS", "N1LS", "N", "BB"};

  int num_threads_;

  IClustFactoryPtr factory_;

 private:

  std::string FormatComputationToJson(const IGraph &graph,
                                      const std::vector<ClusteringInfo> &computation_results,
                                      const unsigned size,
                                      const double density) const;

 public:
  NS2CCSolver(const int num_threads,
              const IClustFactoryPtr &factory);

  std::string solve(const IGraphPtr &graph,
                    const double density,
                    const std::vector<std::string> used_algorithms) const;
};



