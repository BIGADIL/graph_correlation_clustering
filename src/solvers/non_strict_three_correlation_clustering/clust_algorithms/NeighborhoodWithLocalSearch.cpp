#include <climits>
#include <thread>

#include "../../../../include/solvers/non_strict_three_correlation_clustering/clust_algorithms/NeighborhoodWithLocalSearch.hpp"
#include "../../../../include/solvers/non_strict_three_correlation_clustering/common_functions/LocalSearch.hpp"

void non_strict_3cc::NeighborhoodWithLocalSearch::BestNeighborhoodClusteringThreadWorker(const IGraph &graph,
    const unsigned thread_id,
    IClustPtr &local_best_clustering) const {
    unsigned best_distance = UINT_MAX;
    for (unsigned i = thread_id; i < graph.Size(); i += num_threads_) {
        auto part_clustering = neighbor_splitter_.BuildFirstCluster(graph, i);
        for (unsigned j = 0; j < graph.Size(); ++j) {
            if (!part_clustering->IsNonClustered(j)) continue;
            auto tmp_clustering = NeighborSplitter::BuildSecondAndThirdClusters(graph, part_clustering, j);
            tmp_clustering = LocalSearch::ComputeLocalOptimum(
                graph,
                tmp_clustering,
                SECOND_CLUSTER,
                THIRD_CLUSTER);
            if (const auto tmp_distance = tmp_clustering->GetDistanceToGraph(graph); tmp_distance < best_distance) {
                best_distance = tmp_distance;
                local_best_clustering = tmp_clustering;
            }
        }
    }
}

non_strict_3cc::NeighborhoodWithLocalSearch::NeighborhoodWithLocalSearch(unsigned num_threads,
                                                                         const IClustFactoryPtr &
                                                                         clustering_factory) : num_threads_(
        num_threads),
    clustering_factory_(clustering_factory),
    neighbor_splitter_(NeighborSplitter(clustering_factory)) {
}

IClustPtr non_strict_3cc::NeighborhoodWithLocalSearch::getBestNeighborhoodClustering(const IGraph &graph) const {
    std::vector<IClustPtr> local_best_clustering_vector;
    for (unsigned i = 0; i < num_threads_; i++) {
        auto instance = clustering_factory_->CreateClustering(graph.Size());
        local_best_clustering_vector.push_back(instance);
    }
    std::vector<std::thread> thread_vector(num_threads_);
    for (unsigned i = 0; i < num_threads_; i++) {
        thread_vector[i] = std::thread(
            &NeighborhoodWithLocalSearch::BestNeighborhoodClusteringThreadWorker,
            this,
            std::ref(graph),
            i,
            std::ref(local_best_clustering_vector[i])
        );
    }
    for (auto &it: thread_vector) {
        it.join();
    }
    IClustPtr best_neighborhood_clustering = local_best_clustering_vector[0];
    unsigned best_distance = best_neighborhood_clustering->GetDistanceToGraph(graph);
    for (const auto &it: local_best_clustering_vector) {
        if (const auto tmp_distance = it->GetDistanceToGraph(graph); tmp_distance < best_distance) {
            best_distance = tmp_distance;
            best_neighborhood_clustering = it;
        }
    }
    return best_neighborhood_clustering;
}
