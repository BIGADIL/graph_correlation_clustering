#pragma once

#include <vector>
#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"

namespace semi_supervised_2cc {

class LocalSearch {
 private:

 private:
  struct ExcludeVertices {
    unsigned first_cluster_vertex;
    unsigned second_cluster_vertex;

    ExcludeVertices(unsigned vertex, unsigned opposite_vertex)
        : first_cluster_vertex(vertex), second_cluster_vertex(opposite_vertex) {

    }

    [[nodiscard]] bool contain(unsigned i) const {
      return i == first_cluster_vertex || i == second_cluster_vertex;
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
                                                ExcludeVertices exclude_vertices);

  static LocalSearchCandidate FindCandidate(const IGraph &graph,
                                            const std::vector<int> &local_improvement_list,
                                            ExcludeVertices exclude_vertices);

  static std::vector<int> UpdateLocalImprovements(const IGraph &graph,
                                                  const IClustPtr &cur_clustering,
                                                  std::vector<int> &local_improvement_list,
                                                  unsigned vertex,
                                                  ExcludeVertices exclude_vertices);

  static IClustPtr UpdateClustering(IClustPtr &cur_clustering,
                                    unsigned vertex);

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
                                       unsigned first_cluster_vertex,
                                       unsigned second_cluster_vertex);
};
}
