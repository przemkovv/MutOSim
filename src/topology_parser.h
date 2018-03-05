
#pragma once

#include "types.h"

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

// "4": { "compression": { "threshold": 25, "size": 2, "intensity_factor": 1.0} },
struct CompressionRatio {
  Capacity threshold;
  Size size;
  IntensityFactor intensity_factor;
};

struct TrafficClassSettings {
  std::vector<CompressionRatio> compression_ratios{};
};

struct Group {
  GroupName name{};
  Capacity capacity{};
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

Topology parse_topology_config(std::string_view filename);
void dump(const Topology &topology);

} // namespace Config
