
#include "topology.h"

#include "group.h"

Group &Topology::add_group(std::unique_ptr<Group> group)
{
  group->id = GroupId{get_uuid()};
  auto it = groups.emplace(group->get_name(), std::move(group)).first;
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
  groups[from]->add_next_group(groups[to].get());
}

void Topology::attach_source_to_group(const SourceName &source, const GroupName &group)
{
  sources[source]->attach_to_group(groups[group].get());
}

TrafficClass &Topology::add_traffic_class(Intensity source_intensity,
                                          Intensity serve_intensity,
                                          Size size)
{
  auto tc_id = TrafficClassId{traffic_classes.size()};

  return add_traffic_class(tc_id, source_intensity, serve_intensity, size);
}

TrafficClass &Topology::add_traffic_class(TrafficClassId id,
                                          Intensity source_intensity,
                                          Intensity serve_intensity,
                                          Size size)
{
  auto [it, inserted] = traffic_classes.emplace(
      id, TrafficClass{id, source_intensity, serve_intensity, size});

  if (!inserted) {
    println("[Topology] Traffic class with id {} already exists.", id);
  }
  return it->second;
}

void Topology::set_world(gsl::not_null<World *> world)
{
  for (auto &[name, group] : groups) {
    group->set_world(world.get());
    group->set_traffic_classes(traffic_classes);
  }
  for (auto &[name, source] : sources) {
    source->set_world(world.get());
  }
}
std::optional<SourceStream *> Topology::find_source_by_tc_id(TrafficClassId id)
{
  // TODO(PW): use std::find_if
  for (auto &[name, source] : sources) {
    if (source->tc_.id == id) {
      return source.get();
    }
  }
  return {};
}
std::optional<SourceId> Topology::get_source_id(const SourceName &name)
{
  auto it = sources.find(name);
  if (it != sources.end()) {
    return it->second->id;
  }
  return {};
}

const TrafficClass &Topology::get_traffic_class(TrafficClassId id)
{
  return traffic_classes.at(id);
}
const Group &Topology::get_group(const GroupName &group_name)
{
  return *groups[group_name];
}
