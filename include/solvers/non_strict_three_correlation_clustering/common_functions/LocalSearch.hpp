#pragma once

#include <vector>

#include "../../../clustering/IClustering.hpp"
#include "../../../graphs/IGraph.hpp"

namespace non_strict_3cc {
class LocalSearch {
  struct LocalSearchCandidate {
    unsigned vertex;
    int local_improvement;

    LocalSearchCandidate(const unsigned vertex, const int local_improvement)
        : vertex(vertex), local_improvement(local_improvement) {}
  };

  struct FullLocalSearchCandidate {
    unsigned vertex;
    int local_improvement;
    ClusterLabels label;

    FullLocalSearchCandidate(const unsigned vertex, const int local_improvement,
                             const ClusterLabels label)
        : vertex(vertex), local_improvement(local_improvement), label(label) {}
  };

  static std::vector<int> InitLocalImprovements(const IGraph &graph,
                                                const IClustPtr &cur_clustering,
                                                ClusterLabels first_label,
                                                ClusterLabels second_label);

  static std::vector<std::vector<int>> InitLocalImprovements(
      const IGraph &graph, const IClustPtr &cur_clustering);

  static LocalSearchCandidate FindCandidate(
      const IGraph &graph, const std::vector<int> &local_improvement_list,
      const IClustPtr &cur_clustering, ClusterLabels first_label,
      ClusterLabels second_label);

  static FullLocalSearchCandidate FindCandidate(
      const IGraph &graph, const std::vector<std::vector<int>> &cache,
      const IClustPtr &cur_clustering);

  static std::vector<int> UpdateLocalImprovements(
      const IGraph &graph, const IClustPtr &cur_clustering,
      std::vector<int> &local_improvement_list, unsigned vertex,
      ClusterLabels first_label, ClusterLabels second_label);

  static void UpdateLocalImprovements(
      const IGraph &graph, const IClustPtr &cur_clustering,
      std::vector<std::vector<int>> &cache, unsigned vertex,
      ClusterLabels old_label, ClusterLabels new_label);

  static bool IsSkipLabelForVertex(ClusterLabels label, unsigned idx);

  static unsigned GetIdxByLabel(ClusterLabels labels);

  static ClusterLabels GetLabelByIdx(unsigned idx);

 public:
  /**
   * Compute local optimal solution for the source graph.
   *
   * @param graph source graph.
   * @param cur_clustering init clustering.
   * @param first_label
   * @param second_label
   * @return local optimal clustering.
   */
  static IClustPtr ComputeLocalOptimum(const IGraph &graph,
                                       const IClustPtr &cur_clustering,
                                       ClusterLabels first_label,
                                       ClusterLabels second_label);

  static IClustPtr ComputeLocalOptimum(const IGraph &graph,
                                       const IClustPtr &cur_clustering);

  static std::vector<int> ComputeLocalImprovement(
      const IGraph &graph, const IClustPtr &cur_clustering, unsigned vertex);
};
}  // namespace non_strict_3cc
