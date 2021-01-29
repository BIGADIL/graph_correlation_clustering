#pragma once

#include <memory>
#include <vector>

#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace non_strict_2cc {

/**
 * Local search for NS2CC.
 * @see Coleman, Saunderson and Wirth. A Local-Search 2-Approximation for 2-Correlation-Clustering.
 */
class LocalSearch {

 private:
  struct LocalSearchCandidate {
    unsigned vertex;
    int local_improvement;

   public:
    LocalSearchCandidate(unsigned vertex, int local_improvement)
        : vertex(vertex), local_improvement(local_improvement) {

    }
  };

  static std::vector<int> InitLocalImprovements(const IGraph &graph,
                                                const IClustPtr &cur_clustering);

  static LocalSearchCandidate FindCandidate(const IGraph &graph,
                                            const std::vector<int> &local_improvement_list);

  static std::vector<int> UpdateLocalImprovements(const IGraph &graph,
                                                  const IClustPtr &cur_clustering,
                                                  std::vector<int> &local_improvement_list,
                                                  const unsigned vertex);

  static IClustPtr UpdateClustering(IClustPtr &cur_clustering,
                                    const unsigned vertex);

 public:
  /**
   * Compute local optimal solution for the source graph.
   *
   * @param graph source graph.
   * @param cur_clustering init clustering.
   * @return local optimal clustering.
   */
  static IClustPtr ComputeLocalOptimum(const IGraph &graph,
                                       const IClustPtr &cur_clustering);
};
}


