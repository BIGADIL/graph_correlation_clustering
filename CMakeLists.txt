cmake_minimum_required(VERSION 3.14)
project(Graph_correlation_clustering)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(" ")
    message("CMAKE IN DEBUG MODE")
    message(" ")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    message(" ")
    message("CMAKE IN RELEASE MODE")
    message(" ")
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_package( Threads )

add_executable(Graph_correlation_clustering
        main.cpp
        include/graphs/IGraph.hpp
        include/graphs/AdjacencyMatrixGraph.hpp
        src/graphs/factories/ErdosRenyiRandomGraphFactory.cpp
        include/graphs/factories/ErdosRenyiRandomGraphFactory.hpp
        include/clustering/IClustering.hpp
        include/clustering/BinaryClusteringVector.hpp
        src/clustering/BinaryClusteringVector.cpp
        src/algorithms/NeighborhoodAlgorithm.cpp
        include/algorithms/NeighborhoodAlgorithm.hpp
        include/clustering/factories/IClusteringFactory.hpp
        src/clustering/factories/BinaryClusteringFactory.cpp
        include/clustering/factories/BinaryClusteringFactory.hpp
        include/graphs/factories/IGraphFactory.hpp
        include/common/Constants.hpp
        src/algorithms/NeighborSplitter.cpp
        include/algorithms/NeighborSplitter.hpp
        src/algorithms/LocalSearchAlgorithm.cpp
        include/algorithms/LocalSearchAlgorithm.hpp
        src/algorithms/BranchAndBoundAlgorithm.cpp
        include/algorithms/BranchAndBoundAlgorithm.hpp
        src/clustering/BoundAndBranchBinaryClusteringVector.cpp
        include/clustering/BoundAndBranchBinaryClusteringVector.hpp
        include/common/CLusteringLabels.hpp)

target_link_libraries( Graph_correlation_clustering ${CMAKE_THREAD_LIBS_INIT} )