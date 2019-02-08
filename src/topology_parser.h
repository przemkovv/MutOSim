
#pragma once

#include "types/types.h"
#include "types/hash.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace Config
{
enum class SourceType { Poisson, Pascal, Engset };

struct TrafficClass {
  TrafficClassId id{};
  Intensity serve_intensity{};
  Size size{};
  Weight weight{};
  Length max_path_length{};
};

struct Source {
  SourceName name{};
  SourceType type{};
  TrafficClassId tc_id{};
  Count source_number{};
  GroupName attached{};
};

struct CompressionRatio {
  Capacity threshold{};
  Size size{};
  IntensityFactor intensity_factor{};
};

struct TrafficClassSettings {
  std::vector<CompressionRatio> compression_ratios{};
  bool block = false;
};

struct Group {
  GroupName name{};
  std::vector<Capacity> capacities{};
  Layer layer{};
  Intensity intensity_multiplier{};
  std::optional<OverflowPolicyName> overflow_policy{};
  std::vector<GroupName> connected{};
  std::unordered_map<TrafficClassId, TrafficClassSettings> traffic_classess_settings{};
};

struct Topology {
  std::string name{};
  std::unordered_map<TrafficClassId, TrafficClass> traffic_classes{};
  std::vector<Source> sources{};
  std::unordered_map<GroupName, Group> groups{};
};

nlohmann::json load_topology_config(const std::string &filename);
std::pair<Topology, nlohmann::json> parse_topology_config(
    const std::string &filename, const std::vector<std::string> &append_filenames);

void dump(const Topology &topology);

} // namespace Config
