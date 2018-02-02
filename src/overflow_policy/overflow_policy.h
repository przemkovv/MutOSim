#pragma once

#include "load.h"
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

using GroupPtr = observer_ptr<Group>;
//----------------------------------------------------------------------
class OverflowPolicy
{
protected:
  observer_ptr<Group> group_;
  observer_ptr<World> world_;

  static constexpr int overflows_per_layer = 2;
  std::array<int, MaxLayersNumber> count_layers_usage(const Path &path) const;

  std::optional<GroupPtr> fallback_policy();
  std::vector<GroupPtr> get_available_groups(const Load &load);

  template <typename BeginIt, typename EndIt>
  GroupPtr pick_random(BeginIt &&begin, EndIt &&end);

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
class RandomAvailable : public OverflowPolicy
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
