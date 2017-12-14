
#pragma once

#include "group.h"
#include "source_stream.h"

#include <memory>

uint64_t seed();

void add_groups(World &world,
                const std::vector<std::unique_ptr<Group>> &groups);

void add_sources(World &world,
                 const std::vector<std::unique_ptr<SourceStream>> &sources);
