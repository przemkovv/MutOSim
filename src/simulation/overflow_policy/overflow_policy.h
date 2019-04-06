#pragma once

#include "simulation/load.h"
#include "types/types.h"

#include <gsl/gsl>
#include <optional>

namespace Simulation {
struct Group;
struct Load;
class World;

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
  Group *group_ = nullptr;
  World *world_ = nullptr;

  // TODO(PW): extract it to scenario settings
  static constexpr int             overflows_per_layer = 2;
  std::array<int, MaxLayersNumber> count_layers_usage(const Path &path) const;

  std::optional<Group *> fallback_policy();
  std::vector<Group *>   get_available_groups(const Load &load, Layer layer);

  template <typename BeginIt, typename EndIt>
  Group *pick_random(BeginIt &&begin, EndIt &&end);

public:
  OverflowPolicy(Group &group);
  OverflowPolicy(const OverflowPolicy &) = delete;
  OverflowPolicy &operator=(const OverflowPolicy &) = delete;

  virtual std::optional<Group *> find_next_group(const Load &load);
  void                           set_world(World &world);
  virtual ~OverflowPolicy() = default;
};

//----------------------------------------------------------------------
class NoOverflow : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<Group *> find_next_group(const Load &load) override;
};

//----------------------------------------------------------------------
class AlwaysFirst : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<Group *> find_next_group(const Load &load) override;
};

//----------------------------------------------------------------------
class FirstAvailable : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<Group *> find_next_group(const Load &load) override;
};

//----------------------------------------------------------------------
class RandomAvailable : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<Group *> find_next_group(const Load &load) override;
};

//----------------------------------------------------------------------
class HighestFreeCapacity : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<Group *> find_next_group(const Load &load) override;
};

//----------------------------------------------------------------------
class LowestFreeCapacity : public OverflowPolicy
{
public:
  using OverflowPolicy::OverflowPolicy;
  std::optional<Group *> find_next_group(const Load &load) override;
};

} // namespace Simulation
