#include "../../include/common/ExperimentParameters.hpp"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

ExperimentParameters::ExperimentParameters(std::vector<unsigned> graph_size_vector,
                                           std::vector<double> density_vector,
                                           std::vector<std::string> distribution_vector,
                                           std::vector<std::string> algorithms_vector,
                                           std::vector<double> parts,
                                           unsigned num_threads,
                                           unsigned num_graphs) :
    graph_size_vector_(std::move(graph_size_vector)),
    density_vector_(std::move(density_vector)),
    distribution_vector_(std::move(distribution_vector)),
    algorithms_vector_(std::move(algorithms_vector)),
    parts_(std::move(parts)),
    num_threads_(num_threads),
    num_graphs_(num_graphs) {

}

ExperimentParameters ExperimentParameters::readFromConfig(const std::string &path) {
  std::ifstream ifs{path};
  rapidjson::IStreamWrapper isw{ifs};
  rapidjson::Document doc{};
  doc.ParseStream(isw);
  std::string error_message = "Missed or incorrect type of required parameter: ";
  if (!doc.HasMember("graph_size") || !doc["graph_size"].IsArray()) {
    throw std::logic_error(error_message + "graph_size");
  }
  if (!doc.HasMember("algorithms") || !doc["algorithms"].IsArray()) {
    throw std::logic_error(error_message + "algorithms");
  }
  if (!doc.HasMember("num_threads") || !doc["num_threads"].IsUint()) {
    throw std::logic_error(error_message + "num_threads");
  }
  if (!doc.HasMember("num_graphs") || !doc["num_graphs"].IsUint()) {
    throw std::logic_error(error_message + "num_graphs");
  }

  std::vector<unsigned> graph_size_vector;
  const auto &size_array = doc["graph_size"];
  for (unsigned i = 0; i < size_array.Size(); i++) {
    graph_size_vector.push_back(size_array[i].GetInt());
  }
  std::vector<double> density_vector;
  if (doc.HasMember("density") && doc["density"].IsArray()) {
    const auto &density_array = doc["density"];
    for (unsigned i = 0; i < density_array.Size(); i++) {
      density_vector.push_back(density_array[i].GetDouble());
    }
  }
  std::vector<std::string> distribution_vector;
  if (doc.HasMember("distribution") && doc["distribution"].IsArray()) {
    const auto &distribution_array = doc["distribution"];
    for (unsigned i = 0; i < distribution_array.Size(); i++) {
      distribution_vector.emplace_back(distribution_array[i].GetString());
    }
  }
  std::vector<std::string> algorithms_vector;
  const auto &algorithms_array = doc["algorithms"];
  for (unsigned i = 0; i < algorithms_array.Size(); i++) {
    algorithms_vector.emplace_back(algorithms_array[i].GetString());
  }
  std::vector<double> parts;
  if (doc.HasMember("parts") && doc["parts"].IsArray()) {
    const auto &parts_array = doc["parts"];
    for (unsigned i = 0; i < parts_array.Size(); i++) {
      parts.push_back(parts_array[i].GetDouble());
    }
  }
  return {
      graph_size_vector,
      density_vector,
      distribution_vector,
      algorithms_vector,
      parts,
      doc["num_threads"].GetUint(),
      doc["num_graphs"].GetUint()};
}

unsigned int ExperimentParameters::GetNumThreads() const {
  return num_threads_;
}

unsigned int ExperimentParameters::GetNumGraphs() const {
  return num_graphs_;
}

const std::vector<unsigned int> &ExperimentParameters::GetGraphSize() const {
  return graph_size_vector_;
}

const std::vector<double> &ExperimentParameters::GetDensity() const {
  return density_vector_;
}

const std::vector<std::string> &ExperimentParameters::GetAlgorithms() const {
  return algorithms_vector_;
}

const std::vector<double> &ExperimentParameters::GetParts() const {
  return parts_;
}

const std::vector<std::string> &ExperimentParameters::GetDistribution() const {
  return distribution_vector_;
}

