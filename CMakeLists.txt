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
        include/algorithms/non_strict_two_correlation_clustering/NNLSAlgorithmForNS2CC.hpp
        include/algorithms/non_strict_two_correlation_clustering/NSplitterForNS2CC.hpp
        include/algorithms/non_strict_two_correlation_clustering/LSAlgorithmForNS2CC.hpp
        include/algorithms/non_strict_two_correlation_clustering/BBAlgorithmForNS2CC.hpp
        include/algorithms/non_strict_two_correlation_clustering/N1LSAlgorithmForNS2CC.hpp
        include/algorithms/non_strict_two_correlation_clustering/NAlgorithmForNS2CC.hpp

        src/algorithms/non_strict_two_correlation_clustering/NNLSAlgorithmForNS2CC.cpp
        src/algorithms/non_strict_two_correlation_clustering/NSplitterForNS2CC.cpp
        src/algorithms/non_strict_two_correlation_clustering/LSAlgorithmForNS2CC.cpp
        src/algorithms/non_strict_two_correlation_clustering/BBAlgorithmForNS2CC.cpp
        src/algorithms/non_strict_two_correlation_clustering/N1LSAlgorithmForNS2CC.cpp
        src/algorithms/non_strict_two_correlation_clustering/NAlgorithmForNS2CC.cpp
        )

set(S2CC_SCR
        include/algorithms/strict_two_correlation_clustering/NSplitterFor2CC.hpp
        include/algorithms/strict_two_correlation_clustering/LSAlgorithmForS2CC.hpp
        include/algorithms/strict_two_correlation_clustering/NAlgorithmForS2CС.hpp

        src/algorithms/strict_two_correlation_clustering/NSplitterFor2CC.cpp
        src/algorithms/strict_two_correlation_clustering/LSAlgorithmForS2CC.cpp
        src/algorithms/strict_two_correlation_clustering/NAlgorithmForS2CС.cpp
)

add_executable(NS2CCExperiment
        NS2CCExperiment.cpp
        ${COMMON_SRC}
        ${NS2CC_SRC}
        )
target_link_libraries(NS2CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(NS2CCExperiment PUBLIC cxx_std_17)

add_executable(S2CCExperiment
        S2CCExperiment.cpp
        ${COMMON_SRC}
        ${S2CC_SCR}
        )
target_link_libraries(S2CCExperiment ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(S2CCExperiment PUBLIC cxx_std_17)
