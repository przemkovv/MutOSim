
#include "topology.h"

#include "group.h"
#include "source_stream.h"

void Topology::add_group(std::unique_ptr<Group> group)
{
  groups.emplace(group->get_name(), std::move(group));
}

void Topology::add_source(std::unique_ptr<SourceStream> source_stream)
{
  sources.emplace(source_stream->get_name(), std::move(source_stream));
}

void Topology::connect_groups(Name from, Name to)
{
  groups[from]->add_next_group(groups[to].get());
}

void Topology::attach_source_to_group(Name source, Name group)
{
  sources[source]->attach_to_group(groups[group].get());
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
std::optional<SourceStream *> Topology::find_source_by_id(Uuid id)
{
  for (auto & [ name, source ] : sources) {
    if (source->id == id) {
      return source.get();
    }
  }
  return {};
}
