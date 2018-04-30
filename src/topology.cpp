
#include "simulation/source_stream/source_stream.h"
#include "topology.h"

#include "simulation/group.h"

Group &Topology::add_group(std::unique_ptr<Group> group)
{
  group->id = GroupId{get_uuid()};
  auto it = groups.emplace(group->name(), std::move(group)).first;
  return *(it->second.get());
}

SourceStream &Topology::add_source(std::unique_ptr<SourceStream> source_stream)
{
  source_stream->id = SourceId{get_uuid()};
  auto it = sources.emplace(source_stream->get_name(), std::move(source_stream)).first;
  return *(it->second.get());
}

void Topology::connect_groups(const GroupName &from, const GroupName &to)
{
  groups[from]->add_next_group(*groups[to]);
}

void Topology::attach_source_to_group(const SourceName &source, const GroupName &group)
{
  sources[source]->attach_to_group(*groups[group]);
}

TrafficClass &Topology::add_traffic_class(Intensity source_intensity,
                                          Intensity serve_intensity,
                                          Size size,
                                          Length max_path_length)
{
  auto tc_id = TrafficClassId{traffic_classes.size()};

  return add_traffic_class(tc_id, source_intensity, serve_intensity, size,
                           max_path_length);
}

TrafficClass &Topology::add_traffic_class(TrafficClassId id,
                                          Intensity source_intensity,
                                          Intensity serve_intensity,
                                          Size size,
                                          Length max_path_length)
{
  auto [it, inserted] = traffic_classes.emplace(
      id, TrafficClass{id, source_intensity, serve_intensity, size, max_path_length});

  if (!inserted) {
    println("[Topology] Traffic class with id {} already exists.", id);
  }
  return it->second;
}

void Topology::set_world(World& world)
{
  for (auto &[name, group] : groups) {
    std::ignore = name;
    group->set_world(world);
    group->set_traffic_classes(traffic_classes);
  }
  for (auto &[name, source] : sources) {
    std::ignore = name;
    source->set_world(world);
  }
}
std::optional<SourceStream *> Topology::find_source_by_tc_id(TrafficClassId id) const
{
  for (auto &[name, source] : sources) {
    std::ignore = name;
    if (source->tc_.id == id) {
      return source.get();
    }
  }
  return {};
}
std::optional<SourceId> Topology::get_source_id(const SourceName &name) const
{
  auto it = sources.find(name);
  if (it != sources.end()) {
    return it->second->id;
  }
  return {};
}

const TrafficClass &Topology::get_traffic_class(TrafficClassId id) const
{
  return traffic_classes.at(id);
}
const Group &Topology::get_group(const GroupName &group_name) const
{
  return *groups.at(group_name);
}
