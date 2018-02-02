#pragma once

#include "traffic_class.h"
#include "types.h"

#include <boost/container/flat_map.hpp>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>

struct Group;
class World;
class SourceStream;

struct Topology {
  Uuid last_id = 0;

  boost::container::flat_map<GroupName, std::unique_ptr<Group>> groups{};
  boost::container::flat_map<SourceName, std::unique_ptr<SourceStream>> sources{};

  TrafficClasses traffic_classes{};

  Group &add_group(std::unique_ptr<Group> group);
  SourceStream &add_source(std::unique_ptr<SourceStream> source_stream);
  TrafficClass &add_traffic_class(Intensity source_intensity,
                                  Intensity serve_intensity,
                                  Size size,
                                  Length max_path_length = MaxPathLength);

  TrafficClass &add_traffic_class(TrafficClassId id,
                                  Intensity source_intensity,
                                  Intensity serve_intensity,
                                  Size size,
                                  Length max_path_length = MaxPathLength);

  void connect_groups(const GroupName &from, const GroupName &to);

  void attach_source_to_group(const SourceName &source, const GroupName &group);

  void set_world(World &world);

  std::optional<SourceStream *> find_source_by_tc_id(TrafficClassId id) const;
  std::optional<SourceId> get_source_id(const SourceName &name) const;

  Uuid get_uuid() { return ++last_id; }
  const TrafficClass &get_traffic_class(TrafficClassId id) const;
  const Group &get_group(const GroupName &group_name) const;
};
