#pragma once

#include <fstream>
#include <set>
#include <iostream>
#include "../../../include/graphs/factories/StackOverflowGraphFactory.hpp"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"
#include "../../../include/graphs/AdjacencyMatrixGraph.hpp"

StackOverflowGraphFactory::StackOverflowGraphFactory(const std::string &path, const double noise) : noise_(noise) {
  std::ifstream ifs{path};
  rapidjson::IStreamWrapper isw{ifs};
  rapidjson::Document doc{};
  doc.ParseStream(isw);

  for (auto& member: doc.GetObject()) {
    const std::string key = member.name.GetString();
    keys_.emplace_back(key);
    std::vector<std::string> values;
    if (member.value.IsArray()) {
      for (auto &elem: member.value.GetArray()) {
        if (elem.IsString()) {
          values.emplace_back(elem.GetString());
        }
      }
    }
    data_[key] = values;
  }
  dis_ = std::uniform_int_distribution<>(0, keys_.size() - 1);
}

IGraphPtr StackOverflowGraphFactory::CreateGraph(unsigned int size) {
  std::vector<std::vector<bool>> adjacency_matrix(size);
  for (auto &row: adjacency_matrix) {
    row = std::vector<bool>(size);
  }
  std::vector<unsigned> ids;
  std::set<unsigned> ids_set;
  while (ids.size() < size) {
    auto id = dis_(gen_);
    if (!ids_set.contains(id)) {
      ids_set.insert(id);
      ids.emplace_back(id);
    }
  }
  double edges = 0;
  for (unsigned i = 0; i < size; i++) {
    for (unsigned j = i + 1; j < size; j++) {
      auto tags_1 = data_[keys_[ids[i]]];
      auto tags_2 = data_[keys_[ids[j]]];
      bool has_common_element = HasCommonElement(tags_1, tags_2);
      auto noise = noise_generator_(gen_);
      bool swap = noise < noise_;

      if (has_common_element) {
        if (swap) {
          adjacency_matrix[i][j] = adjacency_matrix[j][i] = false;
        } else {
          adjacency_matrix[i][j] = adjacency_matrix[j][i] = true;
          edges++;
        }
      } else {
        if (swap) {
          adjacency_matrix[i][j] = adjacency_matrix[j][i] = true;
          edges++;
        } else {
          adjacency_matrix[i][j] = adjacency_matrix[j][i] = false;
        }
      }
    }
  }
  std::cout << edges / (size * (size - 1) / 2) << std::endl;

  return IGraphPtr(new AdjacencyMatrixGraph(adjacency_matrix));
}

bool StackOverflowGraphFactory::HasCommonElement(const std::vector<std::string> &vec1,
                                                 const std::vector<std::string> &vec2) {
  std::set<std::string> set(vec1.begin(), vec1.end());
  for (const auto& num : vec2) {
    if (set.count(num)) {
      return true;
    }
  }
  return false;
}


