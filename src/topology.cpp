
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
  auto it = sources.emplace(source_stream->get_name(), std::move(source_stream))
                .first;
  return *(it->second.get());
}

void Topology::connect_groups(GroupName from, GroupName to)
{
  groups[from]->add_next_group(groups[to].get());
}

void Topology::attach_source_to_group(SourceName source, GroupName group)
{
  sources[source]->attach_to_group(groups[group].get());
}

void Topology::add_traffic_class(SourceName source_name,
                                 GroupName group_name,
                                 Intensity serve_intensity)
{
  auto &source = sources[source_name];
  auto &tc = traffic_classes.emplace_back(
      TrafficClass{source->id, source->get_intensity(), serve_intensity,
                   source->get_load_size()});
  groups[group_name]->add_traffic_class(tc);
}

void Topology::set_world(gsl::not_null<World *> world)
{
  for (auto & [ name, group ] : groups) {
    group->set_world(world.get());
  }
  for (auto & [ name, source ] : sources) {
    source->set_world(world.get());
  }
}
std::optional<SourceStream *> Topology::find_source_by_id(SourceId id)
{
  for (auto & [ name, source ] : sources) {
    if (source->id == id) {
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
  } else {
    return {};
  }
}
