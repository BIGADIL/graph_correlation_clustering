#pragma once

#include <memory>

#include "../clustering/IClustering.hpp"

using IClusteringPointer = std::shared_ptr<IClustering>;

static const int FIXED_VECTOR_SIZE = 40;