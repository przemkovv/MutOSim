#pragma once

#include "types.h"
#include <optional>
#include <gsl/gsl>

struct Group;
struct Load;

namespace overflow_policy
{
class OverflowPolicy
{
  observer_ptr<Group> group_;

  public:
  OverflowPolicy(gsl::not_null<Group*> group);
  std::optional<observer_ptr<Group>> find_next_group(const Load &load);
};

} // namespace overflow_policy
