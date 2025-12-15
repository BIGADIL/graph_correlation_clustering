#include <fstream>
#include <set>
#include <iostream>
#include <algorithm>
#include <utility>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "../../../include/graphs/factories/TagsGraphFactory.hpp"
#include "../../../include/graphs/AdjacencyMatrixGraph.hpp"

TagsGraphFactory::TagsGraphFactory(const std::string &path, std::string distribution) :
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

IGraphPtr TagsGraphFactory::CreateGraph(unsigned int size) {
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
  double possible = 0, edges = 0;
  for (unsigned i = 0; i < size; i++) {
    for (unsigned j = i + 1; j < size; j++) {
      possible++;
      auto tags_1 = data_[keys_[ids[i]]];
      auto tags_2 = data_[keys_[ids[j]]];
      const auto chance = Chance(tags_1, tags_2);
      bool has_edge;
      if (distribution_.find("0.25") != std::string::npos) {
        has_edge = chance >= 0.25;
      } else if (distribution_.find("0.5") != std::string::npos) {
        has_edge = chance >= 0.5;
      } else if (distribution_.find("0.75") != std::string::npos) {
        has_edge = chance >= 0.75;
      } else {
        throw std::logic_error("Unknown distribution: " + distribution_);
      }
      if (has_edge) {
        edges++;
        adjacency_matrix[i][j] = adjacency_matrix[j][i] = true;
      } else {
        adjacency_matrix[i][j] = adjacency_matrix[j][i] = false;
      }
    }
  }
  std::cout << "density: " << edges / possible << std:: endl;
  return std::make_shared<AdjacencyMatrixGraph>(adjacency_matrix);
}

double TagsGraphFactory::Chance(const std::vector<std::string> &vec1,
                                const std::vector<std::string> &vec2) const {
  std::set set_1(vec1.begin(), vec1.end());
  std::set set_2(vec2.begin(), vec2.end());

  std::set<std::string> intersection_set;
  std::ranges::set_intersection(set_1, set_2,
                                std::inserter(intersection_set, intersection_set.begin())
  );

  std::set<std::string> union_set;
  std::ranges::set_union(set_1, set_2,
                         std::inserter(union_set, union_set.begin())
  );

  if (distribution_.find("jaccard") != std::string::npos) {
    return static_cast<double>(intersection_set.size()) / static_cast<double>(union_set.size());
  }
  if (distribution_.find("cosine") != std::string::npos) {
    return static_cast<double>(intersection_set.size()) / sqrt(static_cast<double>(set_1.size()) * static_cast<double>(set_2.size()));
  }
  if (distribution_.find("dice") != std::string::npos) {
    return 2.0 * static_cast<double>(intersection_set.size()) / (static_cast<double>(set_1.size()) + static_cast<double>(set_2.size()));
  }
  if (distribution_.find("overlap") != std::string::npos) {
    return static_cast<double>(intersection_set.size()) / std::min(static_cast<double>(set_1.size()), static_cast<double>(set_2.size()));
  }
  throw std::logic_error("Unknown method" + distribution_);
}
