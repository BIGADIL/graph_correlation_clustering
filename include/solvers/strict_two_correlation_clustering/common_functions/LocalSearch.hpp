#pragma once

#include <memory>
#include <vector>
#include "../../../clustering/factories/IClusteringFactory.hpp"

namespace strict_2cc {

/**
 * Local search for S2CC.
 * @see Coleman, Saunderson and Wirth. A Local-Search 2-Approximation for 2-Correlation-Clustering.
 */
class LocalSearch {
 private:

 private:
  struct ExcludeVertices {
    unsigned vertex;
    unsigned opposite_vertex;

    ExcludeVertices(unsigned vertex, unsigned opposite_vertex)
        : vertex(vertex), opposite_vertex(opposite_vertex) {

    }

    bool contain(unsigned i) const {
      return i == vertex || i == opposite_vertex;
    }
  };

  struct LocalSearchCandidate {
    unsigned vertex;
    int local_improvement;

   public:
    LocalSearchCandidate(unsigned vertex, int local_improvement)
        : vertex(vertex), local_improvement(local_improvement) {

    }
  };

 private:
  static std::vector<int> InitLocalImprovements(const IGraph &graph,
                                                const IClustPtr &cur_clustering,
                                                const ExcludeVertices exclude_vertices);

  static LocalSearchCandidate FindCandidate(const IGraph &graph,
                                            const std::vector<int> &local_improvement_list,
                                            const ExcludeVertices exclude_vertices);

  static std::vector<int> UpdateLocalImprovements(const IGraph &graph,
                                                  const IClustPtr &cur_clustering,
                                                  std::vector<int> &local_improvement_list,
                                                  const unsigned vertex,
                                                  const ExcludeVertices exclude_vertices);

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
                                       const IClustPtr &cur_clustering,
                                       unsigned vertex,
                                       unsigned opposite_vertex);
};

}
