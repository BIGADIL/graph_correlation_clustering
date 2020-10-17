#include "../../include/common/ExperimentParameters.hpp"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

ExperimentParameters::ExperimentParameters(std::vector<unsigned int> graph_size_vector,
                                           std::vector<double> density_vector,
                                           std::vector<std::string> algorithms_vector) :
    graph_size_vector_(std::move(graph_size_vector)),
    density_vector_(std::move(density_vector)),
    algorithms_vector_(std::move(algorithms_vector)) {

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
  if (!doc.HasMember("density") || !doc["density"].IsArray()) {
    throw std::logic_error(error_message + "density");
  }
  if (!doc.HasMember("algorithms") || !doc["algorithms"].IsArray()) {
    throw std::logic_error(error_message + "density");
  }
  std::vector<unsigned> graph_size_vector;
  const auto &size_array = doc["graph_size"];
  for (unsigned i = 0; i < size_array.Size(); i++) {
    graph_size_vector.push_back(size_array[i].GetInt());
  }
  std::vector<double> density_vector;
  const auto &density_array = doc["density"];
  for (unsigned i = 0; i < density_array.Size(); i++) {
    density_vector.push_back(density_array[i].GetDouble());
  }
  std::vector<std::string> algorithms_vector;
  const auto &algorithms_array = doc["algorithms"];
  for (unsigned i = 0; i < algorithms_array.Size(); i++) {
    algorithms_vector.emplace_back(algorithms_array[i].GetString());
  }
  return ExperimentParameters(graph_size_vector, density_vector, algorithms_vector);
}
