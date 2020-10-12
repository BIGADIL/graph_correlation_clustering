cmake_minimum_required(VERSION 3.8)
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
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -lstdc++fs")

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
        src/algorithms/non_strict_two_correlation_clustering/NWithLSAlgorithm.cpp
        include/algorithms/non_strict_two_correlation_clustering/NWithLSAlgorithm.hpp
        include/clustering/factories/IClusteringFactory.hpp
        src/clustering/factories/BinaryClusteringFactory.cpp
        include/clustering/factories/BinaryClusteringFactory.hpp
        include/graphs/factories/IGraphFactory.hpp
        src/algorithms/non_strict_two_correlation_clustering/NSplitter.cpp
        include/algorithms/non_strict_two_correlation_clustering/NSplitter.hpp
        src/algorithms/non_strict_two_correlation_clustering/LSAlgorithm.cpp
        include/algorithms/non_strict_two_correlation_clustering/LSAlgorithm.hpp
        src/algorithms/non_strict_two_correlation_clustering/BBAlgorithm.cpp
        include/algorithms/non_strict_two_correlation_clustering/BBAlgorithm.hpp
        src/clustering/BBBinaryClusteringVector.cpp
        include/clustering/BBBinaryClusteringVector.hpp
        include/common/CLusteringLabels.hpp src/graphs/AdjacencyMatrixGraph.cpp)

target_link_libraries( Graph_correlation_clustering ${CMAKE_THREAD_LIBS_INIT} )

target_compile_features(Graph_correlation_clustering PUBLIC cxx_std_17)