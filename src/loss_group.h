#pragma once

#include "types.h"
#include "stats.h"
#include "load.h"

#include <fmt/format.h>

struct LossGroup {
  const GroupName name_;

  std::unordered_map<SourceId, LostServedStats> &served_by_source;

  LossGroup(GroupName name,
            std::unordered_map<SourceId, LostServedStats> &served_by_source);
  bool serve(Load load);
};

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&format_str,
                const LossGroup &loss_group);
