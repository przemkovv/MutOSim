#pragma once

#include "types.h"
#include <gsl/gsl>
#include <optional>

struct Group;
struct Load;
class World;

namespace overflow_policy
{
class OverflowPolicy;
class NoOverflow;
class AlwaysFirst;
class FirstAvailable;
class RandomAvailable;
class HighestFreeCapacity;
class LowestFreeCapacity;

using Default = NoOverflow;

//----------------------------------------------------------------------
class OverflowPolicy
{
protected:
  observer_ptr<Group> group_;
  observer_ptr<World> world_;

public:
  OverflowPolicy(gsl::not_null<Group *> group);
  virtual std::optional<observer_ptr<Group>> find_next_group(const Load &load);
  void set_world(gsl::not_null<World *> world);
  virtual ~OverflowPolicy() = default;
};

//----------------------------------------------------------------------
class NoOverflow : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};

//----------------------------------------------------------------------
class AlwaysFirst : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};

//----------------------------------------------------------------------
class FirstAvailable : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};

//----------------------------------------------------------------------
class RandomAvailable: public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};

//----------------------------------------------------------------------
class HighestFreeCapacity : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};
;
//----------------------------------------------------------------------
class LowestFreeCapacity : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<observer_ptr<Group>> find_next_group(const Load &load) override;
};
;

} // namespace overflow_policy
