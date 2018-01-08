#pragma once

#include "source_stream/source_stream.h"
#include "types.h"

#include <gsl/gsl>
#include <memory>
#include <optional>
#include <unordered_map>

struct Group;
class SourceStream;
class World;

struct Topology {
  Uuid last_id = 0;

  std::unordered_map<GroupName, std::unique_ptr<Group>> groups;
  std::unordered_map<SourceName, std::unique_ptr<SourceStream>> sources;

  TrafficClasses traffic_classes;

  Group &add_group(std::unique_ptr<Group> group);
  SourceStream &add_source(std::unique_ptr<SourceStream> source_stream);
  TrafficClass &add_traffic_class(Intensity source_intensity,
                                  Intensity serve_intensity,
                                  Size size);

  void connect_groups(GroupName from, GroupName to);

  void attach_source_to_group(SourceName source, GroupName group);

  void set_world(gsl::not_null<World *> world);

  std::optional<SourceStream *> find_source_by_tc_id(TrafficClassId id);
  std::optional<SourceId> get_source_id(const SourceName &name);

  Uuid get_uuid() { return ++last_id; }
  const TrafficClass& get_traffic_class(TrafficClassId id);
  const Group& get_group(const GroupName & group_name);
};
