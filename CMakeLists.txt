cmake_minimum_required(VERSION 3.8)
project(Graph_correlation_clustering)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(" ")
    message("CMAKE IN DEBUG MODE")
    message(" ")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    message(" ")
    message("CMAKE IN RELEASE MODE")
    message(" ")
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -lstdc++fs")

find_package(Threads)

set(COMMON_SRC
        include/graphs/IGraph.hpp
        include/graphs/AdjacencyMatrixGraph.hpp
        include/graphs/factories/IGraphFactory.hpp
        include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp
        include/clustering/IClustering.hpp
        include/clustering/BinaryClusteringVector.hpp
        include/clustering/factories/IClusteringFactory.hpp
        include/clustering/factories/BinaryClusteringFactory.hpp
        include/clustering/BBBinaryClusteringVector.hpp
        include/common/ClusteringLabels.hpp

        src/graphs/AdjacencyMatrixGraph.cpp
        src/graphs/factories/ErdosRenyiRandomGraphFactory.cpp
        src/clustering/BinaryClusteringVector.cpp
        src/clustering/factories/BinaryClusteringFactory.cpp
        src/clustering/BBBinaryClusteringVector.cpp
        )

set(NS2CC_SRC
        include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithManyLocalSearches.hpp
        include/solvers/non_strict_two_correlation_clustering/common_functions/NeighborSplitter.hpp
        include/solvers/non_strict_two_correlation_clustering/common_functions/LocalSearch.hpp
        include/solvers/non_strict_two_correlation_clustering/clust_algoritms/BranchAndBounds.hpp
        include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithOneLocalSearch.hpp
        include/solvers/non_strict_two_correlation_clustering/clust_algoritms/Neighborhood.hpp
        include/solvers/non_strict_two_correlation_clustering/NonStrict2CCSolver.hpp

        src/solvers/non_strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.cpp
        src/solvers/non_strict_two_correlation_clustering/common_functions/NeighborSplitter.cpp
        src/solvers/non_strict_two_correlation_clustering/common_functions/LocalSearch.cpp
        src/solvers/non_strict_two_correlation_clustering/clust_algorithms/BranchAndBounds.cpp
        src/solvers/non_strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.cpp
        src/solvers/non_strict_two_correlation_clustering/clust_algorithms/Neighborhood.cpp
        src/solvers/non_strict_two_correlation_clustering/NonStrict2CCSolver.cpp
        )

set(S2CC_SCR
        include/solvers/strict_two_correlation_clustering/common_functions/NeighborSplitter.hpp
        include/solvers/strict_two_correlation_clustering/common_functions/LocalSearch.hpp
        include/solvers/strict_two_correlation_clustering/clust_algorithms/Neighborhood.hpp
        include/solvers/strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.hpp
        include/solvers/strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.hpp
        include/solvers/strict_two_correlation_clustering/clust_algorithms/BranchAndBounds.hpp
        include/solvers/strict_two_correlation_clustering/Strict2CCSolver.hpp

        src/solvers/strict_two_correlation_clustering/common_functions/NeighborSplitter.cpp
        src/solvers/strict_two_correlation_clustering/common_functions/LocalSearch.cpp
        src/solvers/strict_two_correlation_clustering/clust_algorithms/Neighborhood.cpp
        src/solvers/strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.cpp
        src/solvers/strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.cpp
        src/solvers/strict_two_correlation_clustering/clust_algorithms/BranchAndBounds.cpp
        src/solvers/strict_two_correlation_clustering/Strict2CCSolver.cpp
        )

set(2SCC_SRC
        include/solvers/two_semi_supervised_correlation_clustering/common_functions/NeighborSplitter.hpp
        include/solvers/two_semi_supervised_correlation_clustering/common_functions/LocalSearch.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/Neighborhood.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BranchAndBounds.hpp
        include/solvers/two_semi_supervised_correlation_clustering/SemiSupervised2CCSolver.hpp

        src/solvers/two_semi_supervised_correlation_clustering/common_functions/NeighborSplitter.cpp
        src/solvers/two_semi_supervised_correlation_clustering/common_functions/LocalSearch.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/Neighborhood.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BranchAndBounds.cpp
        src/solvers/two_semi_supervised_correlation_clustering/SemiSupervised2CCSolver.cpp
        )

add_executable(NonStrict2CCExperiment
        NonStrict2CCExperiment.cpp
        ${COMMON_SRC}
        ${NS2CC_SRC}
        )
target_link_libraries(NonStrict2CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(NonStrict2CCExperiment PUBLIC cxx_std_17)

add_executable(Strict2CCExperiment
        Strict2CCExperiment.cpp
        ${COMMON_SRC}
        ${S2CC_SCR}
        )
target_link_libraries(Strict2CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(Strict2CCExperiment PUBLIC cxx_std_17)

add_executable(SemiSupervised2CCExperiment
        SemiSupervised2CCExperiment.cpp
        ${COMMON_SRC}
        ${2SCC_SRC}
        )
target_link_libraries(SemiSupervised2CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(SemiSupervised2CCExperiment PUBLIC cxx_std_17)