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
protected:
  observer_ptr<Group> group_;

public:
  OverflowPolicy(gsl::not_null<Group *> group);
  virtual std::optional<observer_ptr<Group>> find_next_group(const Load &load);

  virtual ~OverflowPolicy() = default;
};

class NoOverflow : public OverflowPolicy
{

public:
  NoOverflow(gsl::not_null<Group *> group) : OverflowPolicy(group) {}
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};

class AlwaysFirst : public OverflowPolicy
{

public:
  AlwaysFirst(gsl::not_null<Group *> group) : OverflowPolicy(group) {}
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};
class FirstAvailable : public OverflowPolicy
{

public:
  FirstAvailable(gsl::not_null<Group *> group) : OverflowPolicy(group) {}
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};

} // namespace overflow_policy
