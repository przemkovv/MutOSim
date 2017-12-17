#pragma once

#include "types.h"

#include <map>
#include <memory>
#include <gsl/gsl>

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

  void set_world(gsl::not_null<World *>world);
};
