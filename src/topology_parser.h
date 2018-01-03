
#pragma once

#include "types.h"

#include <string_view>

namespace Config
{
enum class SourceType { Poisson, Pascal, Engset };

struct TrafficClass {
  TrafficClassId id;
  Intensity serve_intensity;
  Size size;
  Weight weight;
};

struct Source {
  SourceType type;
  TrafficClass id;
  GroupName attached;
};

struct Group {
  GroupName name;
  Capacity capacity;
};

struct Topology {
  std::vector<TrafficClass> traffic_classes;
  std::vector<Source> sources;
  std::vector<Group> groups;
};

Topology parse_topology_config(std::string_view filename);

} // namespace Config
