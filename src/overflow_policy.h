#pragma once

#include "types.h"
#include <gsl/gsl>
#include <optional>

struct Group;
struct Load;

namespace overflow_policy
{
class OverflowPolicy
{
  observer_ptr<Group> group_;

public:
  OverflowPolicy(gsl::not_null<Group *> group);
  virtual std::optional<observer_ptr<Group>> find_next_group(const Load &load);

  virtual ~OverflowPolicy() = default;
};

class AlwaysFirst : OverflowPolicy
{
  observer_ptr<Group> group_;

public:
  AlwaysFirst(gsl::not_null<Group *> group) : OverflowPolicy(group) {}
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};
class FirstAvailable : OverflowPolicy
{
  observer_ptr<Group> group_;

public:
  FirstAvailable(gsl::not_null<Group *> group) : OverflowPolicy(group) {}
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};

} // namespace overflow_policy
