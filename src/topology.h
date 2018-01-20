#pragma once

#include "source_stream/source_stream.h"
#include "types.h"

#include <gsl/gsl>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>

struct Group;
class SourceStream;
class World;

struct Topology {
  Uuid last_id = 0;

  std::map<GroupName, std::unique_ptr<Group>> groups;
  std::map<SourceName, std::unique_ptr<SourceStream>> sources;

  TrafficClasses traffic_classes;

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

  void set_world(gsl::not_null<World *> world);

  std::optional<SourceStream *> find_source_by_tc_id(TrafficClassId id) const;
  std::optional<SourceId> get_source_id(const SourceName &name) const;

  Uuid get_uuid() { return ++last_id; }
  const TrafficClass &get_traffic_class(TrafficClassId id) const;
  const Group &get_group(const GroupName &group_name) const;
};
