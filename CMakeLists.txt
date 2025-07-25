cmake_minimum_required(VERSION 3.8)
project(graph_correlation_clustering)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(" ")
    message("CMAKE IN DEBUG MODE")
    message(" ")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    message(" ")
    message("CMAKE IN RELEASE MODE")
    message(" ")
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -lstdc++fs")

find_package(Threads)

include(ExternalProject)
include("${CMAKE_SOURCE_DIR}/vendor/rapidjson.cmake")
include_directories(${RAPIDJSON_INCLUDE_DIR})

set(COMMON_SRC
        include/graphs/IGraph.hpp
        include/graphs/AdjacencyMatrixGraph.hpp
        include/graphs/factories/IGraphFactory.hpp
        include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp
        include/graphs/factories/StackOverflowGraphFactory.hpp
        include/clustering/IClustering.hpp
        include/clustering/BinaryClusteringVector.hpp
        include/clustering/factories/IClusteringFactory.hpp
        include/clustering/factories/BinaryClusteringFactory.hpp
        include/clustering/BBBinaryClusteringVector.hpp
        include/common/ClusteringLabels.hpp
        include/clustering/TripleClusteringVector.hpp
        include/clustering/factories/TripleClusteringFactory.hpp
        include/common/ExperimentParameters.hpp

        include/clustering/BBTripleClusteringVector.hpp
        src/graphs/AdjacencyMatrixGraph.cpp
        src/graphs/factories/ErdosRenyiRandomGraphFactory.cpp
        src/graphs/factories/StackOverflowGraphFactory.cpp
        src/clustering/BinaryClusteringVector.cpp
        src/clustering/factories/BinaryClusteringFactory.cpp
        src/clustering/BBBinaryClusteringVector.cpp
        src/clustering/TripleClusteringVector.cpp
        src/clustering/factories/TripleClusteringFactory.cpp
        src/clustering/BBTripleClusteringVector.cpp
        src/common/ExperimentParameters.cpp
        )

set(NS2CC_SRC
        include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithManyLocalSearches.hpp
        include/solvers/non_strict_two_correlation_clustering/common_functions/NeighborSplitter.hpp
        include/solvers/non_strict_two_correlation_clustering/common_functions/LocalSearch.hpp
        include/solvers/non_strict_two_correlation_clustering/clust_algoritms/BranchAndBounds.hpp
        include/solvers/non_strict_two_correlation_clustering/clust_algoritms/NeighborhoodWithOneLocalSearch.hpp
        include/solvers/non_strict_two_correlation_clustering/clust_algoritms/Neighborhood.hpp
        include/solvers/non_strict_two_correlation_clustering/NonStrict2CCSolver.hpp
        include/solvers/non_strict_two_correlation_clustering/clust_algoritms/BrutForce.hpp

        src/solvers/non_strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.cpp
        src/solvers/non_strict_two_correlation_clustering/common_functions/NeighborSplitter.cpp
        src/solvers/non_strict_two_correlation_clustering/common_functions/LocalSearch.cpp
        src/solvers/non_strict_two_correlation_clustering/clust_algorithms/BranchAndBounds.cpp
        src/solvers/non_strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.cpp
        src/solvers/non_strict_two_correlation_clustering/clust_algorithms/Neighborhood.cpp
        src/solvers/non_strict_two_correlation_clustering/NonStrict2CCSolver.cpp
        src/solvers/non_strict_two_correlation_clustering/clust_algorithms/BrutForce.cpp
        )

set(S2CC_SCR
        include/solvers/strict_two_correlation_clustering/common_functions/NeighborSplitter.hpp
        include/solvers/strict_two_correlation_clustering/common_functions/LocalSearch.hpp
        include/solvers/strict_two_correlation_clustering/clust_algorithms/Neighborhood.hpp
        include/solvers/strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.hpp
        include/solvers/strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.hpp
        include/solvers/strict_two_correlation_clustering/clust_algorithms/BranchAndBounds.hpp
        include/solvers/strict_two_correlation_clustering/Strict2CCSolver.hpp
        include/solvers/strict_two_correlation_clustering/clust_algorithms/BrutForce.hpp

        src/solvers/strict_two_correlation_clustering/common_functions/NeighborSplitter.cpp
        src/solvers/strict_two_correlation_clustering/common_functions/LocalSearch.cpp
        src/solvers/strict_two_correlation_clustering/clust_algorithms/Neighborhood.cpp
        src/solvers/strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.cpp
        src/solvers/strict_two_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.cpp
        src/solvers/strict_two_correlation_clustering/clust_algorithms/BranchAndBounds.cpp
        src/solvers/strict_two_correlation_clustering/Strict2CCSolver.cpp
        src/solvers/strict_two_correlation_clustering/clust_algorithms/BrutForce.cpp
        )

set(2SCC_SRC
        include/solvers/two_semi_supervised_correlation_clustering/common_functions/NeighborSplitter.hpp
        include/solvers/two_semi_supervised_correlation_clustering/common_functions/LocalSearch.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/Neighborhood.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BranchAndBounds.hpp
        include/solvers/two_semi_supervised_correlation_clustering/SemiSupervised2CCSolver.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BrutForce.hpp
        include/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodOfPreClusteringVertices.hpp

        src/solvers/two_semi_supervised_correlation_clustering/common_functions/NeighborSplitter.cpp
        src/solvers/two_semi_supervised_correlation_clustering/common_functions/LocalSearch.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/Neighborhood.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BranchAndBounds.cpp
        src/solvers/two_semi_supervised_correlation_clustering/SemiSupervised2CCSolver.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/BrutForce.cpp
        src/solvers/two_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodOfPreClusteringVertices.cpp
        )

set(NS3CC_SRC
        include/solvers/non_strict_three_correlation_clustering/common_functions/NeighborSplitter.hpp
        include/solvers/non_strict_three_correlation_clustering/common_functions/LocalSearch.hpp
        include/solvers/non_strict_three_correlation_clustering/clust_algorithms/NeighborhoodWithLocalSearch.hpp
        include/solvers/non_strict_three_correlation_clustering/NonStrict3CcSolver.hpp
        include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhood.hpp
        include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithLocalSearch.hpp
        include/solvers/non_strict_three_correlation_clustering/clust_algorithms/BranchAndBounds.hpp
        include/solvers/non_strict_three_correlation_clustering/clust_algorithms/BrutForce.hpp
        include/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithManyLocalSearches.hpp
        include/solvers/non_strict_three_correlation_clustering/genetic_algorithms/IGeneticAlgorithm.hpp
        include/solvers/non_strict_three_correlation_clustering/genetic_algorithms/GeneticAlgorithm.hpp

        src/solvers/non_strict_three_correlation_clustering/common_functions/NeighborSplitter.cpp
        src/solvers/non_strict_three_correlation_clustering/common_functions/LocalSearch.cpp
        src/solvers/non_strict_three_correlation_clustering/clust_algorithms/NeighborhoodWithLocalSearch.cpp
        src/solvers/non_strict_three_correlation_clustering/NonStrict3CcSolver.cpp
        src/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhood.cpp
        src/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithLocalSearch.cpp
        src/solvers/non_strict_three_correlation_clustering/clust_algorithms/BranchAndBounds.cpp
        src/solvers/non_strict_three_correlation_clustering/clust_algorithms/BrutForce.cpp
        src/solvers/non_strict_three_correlation_clustering/clust_algorithms/TwoVerticesNeighborhoodWithManyLocalSearches.cpp
        src/solvers/non_strict_three_correlation_clustering/genetic_algorithms/GeneticAlgorithm.cpp
        )

set(2SSCC_SRC
        include/solvers/two_set_semi_supervised_correlation_clustering/common_functions/NeighborSplitter.hpp
        include/solvers/two_set_semi_supervised_correlation_clustering/common_functions/LocalSearch.hpp
        include/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/Neighborhood.hpp
        include/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.hpp
        include/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.hpp
        include/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/BranchAndBounds.hpp
        include/solvers/two_set_semi_supervised_correlation_clustering/SetSemiSupervised2CCSolver.hpp
        include/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/BrutForce.hpp
        include/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodOfPreClusteringVertices.hpp

        src/solvers/two_set_semi_supervised_correlation_clustering/common_functions/NeighborSplitter.cpp
        src/solvers/two_set_semi_supervised_correlation_clustering/common_functions/LocalSearch.cpp
        src/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/Neighborhood.cpp
        src/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithManyLocalSearches.cpp
        src/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodWithOneLocalSearch.cpp
        src/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/BranchAndBounds.cpp
        src/solvers/two_set_semi_supervised_correlation_clustering/SetSemiSupervised2CCSolver.cpp
        src/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/BrutForce.cpp
        src/solvers/two_set_semi_supervised_correlation_clustering/clust_algorithms/NeighborhoodOfPreClusteringVertices.cpp
        )

add_executable(NonStrict2CCExperiment
        NonStrict2CCExperiment.cpp
        ${COMMON_SRC}
        ${NS2CC_SRC}
        )
target_link_libraries(NonStrict2CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(NonStrict2CCExperiment PUBLIC cxx_std_20)
add_dependencies(NonStrict2CCExperiment rapidjson)

add_executable(Strict2CCExperiment
        Strict2CCExperiment.cpp
        ${COMMON_SRC}
        ${S2CC_SCR}
        )
target_link_libraries(Strict2CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(Strict2CCExperiment PUBLIC cxx_std_20)
add_dependencies(Strict2CCExperiment rapidjson)

add_executable(SemiSupervised2CCExperiment
        SemiSupervised2CCExperiment.cpp
        ${COMMON_SRC}
        ${2SCC_SRC}
        )
target_link_libraries(SemiSupervised2CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(SemiSupervised2CCExperiment PUBLIC cxx_std_20)
add_dependencies(SemiSupervised2CCExperiment rapidjson)

add_executable(NonStrict3CCExperiment
        NonStrict3CCExperiment.cpp
        ${COMMON_SRC}
        ${NS3CC_SRC}
        )
target_link_libraries(NonStrict3CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(NonStrict3CCExperiment PUBLIC cxx_std_20)
add_dependencies(NonStrict3CCExperiment rapidjson)

add_executable(NonStrict3CCExperimentSO
        NonStrict3CCExperimentSO.cpp
        ${COMMON_SRC}
        ${NS3CC_SRC}
        )
target_link_libraries(NonStrict3CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(NonStrict3CCExperimentSO PUBLIC cxx_std_20)
add_dependencies(NonStrict3CCExperimentSO rapidjson)

add_executable(SetSemiSupervised2CCExperiment
        SetSemiSupervised2CCExperiment.cpp
        ${COMMON_SRC}
        ${2SSCC_SRC}
        )
target_link_libraries(SetSemiSupervised2CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(SetSemiSupervised2CCExperiment PUBLIC cxx_std_20)
add_dependencies(SetSemiSupervised2CCExperiment rapidjson)

add_executable(Genetic_3
        Genetic_3.cpp
        ${COMMON_SRC}
        ${NS3CC_SRC}
        )
target_link_libraries(Genetic_3 ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(Genetic_3 PUBLIC cxx_std_20)
add_dependencies(Genetic_3 rapidjson)
