#pragma once

#include "types.h"

#include <gsl/gsl>
#include <map>
#include <memory>
#include <optional>

struct Group;
class SourceStream;
class World;

struct Topology {
  std::map<Name, std::unique_ptr<Group>> groups;
  std::map<Name, std::unique_ptr<SourceStream>> sources;

  void add_group(std::unique_ptr<Group> group);

  void add_source(std::unique_ptr<SourceStream> source_stream);

  void connect_groups(Name from, Name to);

  void attach_source_to_group(Name source, Name group);

  void set_world(gsl::not_null<World *> world);

  std::optional<SourceStream *> find_source_by_id(Uuid id);
};
