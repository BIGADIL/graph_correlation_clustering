#pragma once

#include <vector>
#include "../../../graphs/IGraph.hpp"
#include "../../../clustering/IClustering.hpp"

namespace non_strict_3cc {

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
                                                const IClustPtr &cur_clustering,
                                                ClusterLabels first_label,
                                                ClusterLabels second_label);

  static LocalSearchCandidate FindCandidate(const IGraph &graph,
                                            const std::vector<int> &local_improvement_list,
                                            const IClustPtr &cur_clustering,
                                            ClusterLabels first_label,
                                            ClusterLabels second_label);

  static std::vector<int> UpdateLocalImprovements(const IGraph &graph,
                                                  const IClustPtr &cur_clustering,
                                                  std::vector<int> &local_improvement_list,
                                                  unsigned vertex,
                                                  ClusterLabels first_label,
                                                  ClusterLabels second_label);



  static bool IsSkipLabelForVertex(ClusterLabels label, unsigned idx);

  static unsigned GetIdxByLabel(ClusterLabels labels);

  static ClusterLabels GetLabelByIdx(unsigned idx);

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
                                       ClusterLabels first_label,
                                       ClusterLabels second_label);

  static IClustPtr ComputeLocalOptimum(const IGraph &graph,
                                       const IClustPtr &cur_clustering);

  static std::vector<int> ComputeLocalImprovement(const IGraph &graph,
                                                  const IClustPtr &cur_clustering,
                                                  unsigned vertex);
};

}
