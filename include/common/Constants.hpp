#pragma once

#include <memory>

#include "../clustering/IClustering.hpp"

using IClusteringPointer = std::shared_ptr<IClustering>;

static const int FIXED_VECTOR_SIZE = 53;

enum ClusterLabels {
  NON_CLUSTERED = -1,
  FIRST_CLUSTER = 0,
  SECOND_CLUSTER = 1
};