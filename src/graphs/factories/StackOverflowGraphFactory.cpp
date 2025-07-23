#pragma once

#include <fstream>
#include <set>
#include <iostream>
#include <algorithm>
#include <utility>
#include "../../../include/graphs/factories/StackOverflowGraphFactory.hpp"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"
#include "../../../include/graphs/AdjacencyMatrixGraph.hpp"

StackOverflowGraphFactory::StackOverflowGraphFactory(const std::string &path, std::string distribution) :
    distribution_(std::move(distribution)) {
  std::ifstream ifs{path};
  rapidjson::IStreamWrapper isw{ifs};
  rapidjson::Document doc{};
  doc.ParseStream(isw);

  for (auto &member: doc.GetObject()) {
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
}

IGraphPtr StackOverflowGraphFactory::CreateGraph(unsigned int size) {
  std::vector<std::vector<bool>> adjacency_matrix(size);
  for (auto &row: adjacency_matrix) {
    row = std::vector<bool>(size);
  }
  std::vector<unsigned> ids;
  std::set<unsigned> ids_set;
  std::uniform_int_distribution<> dis_int(0, keys_.size() - 1);
  std::uniform_real_distribution dis_real(0.0, 1.0);
  while (ids.size() < size) {
    auto id = dis_int(gen_);
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
      auto chance = Chance(tags_1, tags_2);
      bool has_edge;
      if (distribution_ == "probs") {
        auto proba = dis_real(gen_);
        has_edge = proba <= chance;
      } else if (distribution_ == "threshold") {
        has_edge = chance >= 0.5;
      } else {
        throw std::logic_error("unknown distribution: " + distribution_);
      }
      if (has_edge) {
        adjacency_matrix[i][j] = adjacency_matrix[j][i] = true;
        edges++;
      } else {
        adjacency_matrix[i][j] = adjacency_matrix[j][i] = false;
      }
    }
  }
  std::cout << edges / (size * (size - 1) / 2) << std::endl;

  return IGraphPtr(new AdjacencyMatrixGraph(adjacency_matrix));
}

double StackOverflowGraphFactory::Chance(const std::vector<std::string> &vec1,
                                         const std::vector<std::string> &vec2) {
  std::set<std::string> set_1(vec1.begin(), vec1.end());
  std::set<std::string> set_2(vec2.begin(), vec2.end());

  std::set<std::string> intersection_set;
  std::set_intersection(
      set_1.begin(), set_1.end(),
      set_2.begin(), set_2.end(),
      std::inserter(intersection_set, intersection_set.begin())
  );

  std::set<std::string> union_set;
  std::set_union(
      set_1.begin(), set_1.end(),
      set_2.begin(), set_2.end(),
      std::inserter(union_set, union_set.begin())
  );

  auto chance = (double) intersection_set.size() / (double) union_set.size();
  return chance;
}


