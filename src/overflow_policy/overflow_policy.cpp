
#include "overflow_policy.h"

#include "group.h"
#include "load.h"
#include "source_stream/source_stream.h"
#include "utils.h"

#include <type_traits>

namespace overflow_policy
{
OverflowPolicy::OverflowPolicy(Group &group) : group_(&group)
{
}

void OverflowPolicy::set_world(World &world)
{
  world_ = &world;
}

template <typename BeginIt, typename EndIt>
Group *OverflowPolicy::pick_random(BeginIt &&begin, EndIt &&end)
{
  return get_random_element(begin, end, world_->get_random_engine());
}

std::array<int, MaxLayersNumber>
OverflowPolicy::count_layers_usage(const Path &path) const
{
  std::array<int, MaxLayersNumber> layers_usage_counter{0};
  for (const auto &group : path) {
    layers_usage_counter[group->layer_]++;
  }
  return layers_usage_counter;
}
std::vector<Group *> OverflowPolicy::get_available_groups(const Load &load, Layer layer)
{
  const auto layers_usage = count_layers_usage(load.served_by);
  std::vector<Group *> available_groups;

  available_groups.reserve(group_->next_groups_.size());
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [&](const auto &group) {
                 return layers_usage[group->layer_] < overflows_per_layer &&
                        group->layer_ == layer && group->can_serve(load.tc_id).first &&
                        !contains(load.served_by, group);
               });
  return available_groups;
}

std::optional<Group *> OverflowPolicy::fallback_policy()
{
  std::vector<Group *> available_groups;

  available_groups.reserve(group_->next_groups_.size());
  auto current_layer = group_->layer_;
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [current_layer](const auto &group) {
                 return group->layer_ > current_layer;
               });

  if (!available_groups.empty()) {
    return pick_random(begin(available_groups), end(available_groups));
  }
  return {};
}

//----------------------------------------------------------------------
std::optional<Group *> NoOverflow::find_next_group(const Load & /* load */)
{
  return {};
}

//----------------------------------------------------------------------
std::optional<Group *> FirstAvailable::find_next_group(const Load &load)
{
  for (const auto &next_group : group_->next_groups_) {
    if (std::find(std::begin(load.served_by), std::end(load.served_by), next_group) ==
        std::end(load.served_by)) {
      if (auto [is_served, compression] = next_group->can_serve(load.tc_id); is_served) {
        std::ignore = compression;
        return next_group;
      }
    }
  }
  return {};
}

//----------------------------------------------------------------------
std::optional<Group *> AlwaysFirst::find_next_group(const Load &load)
{
  for (const auto &next_group : group_->next_groups_) {
    if (std::find(std::begin(load.served_by), std::end(load.served_by), next_group) ==
        std::end(load.served_by)) {
      return next_group;
    }
  }
  return {};
}

//----------------------------------------------------------------------
std::optional<Group *> OverflowPolicy::find_next_group(const Load &load)
{
  for (const auto &next_group : group_->next_groups_) {
    if (std::find(std::begin(load.served_by), std::end(load.served_by), next_group) ==
        std::end(load.served_by)) {
      return next_group;
    }
  }
  return {};
}

//----------------------------------------------------------------------
std::optional<Group *> RandomAvailable::find_next_group(const Load &load)
{
  auto available_groups = get_available_groups(load, group_->layer_);

  if (!available_groups.empty()) {
    if (available_groups.size() == 1) {
      return available_groups.front();
    }

    return pick_random(begin(available_groups), end(available_groups));
  }
  return fallback_policy();
}
//----------------------------------------------------------------------
std::optional<Group *> HighestFreeCapacity::find_next_group(const Load &load)
{
  auto available_groups = get_available_groups(load, group_->layer_);

  if (!available_groups.empty()) {
    sort(begin(available_groups), end(available_groups),
         [](const auto &group_ptr1, const auto &group_ptr2) {
           return group_ptr1->free_capacity() > group_ptr2->free_capacity();
         });

    auto &g1 = available_groups.front();
    auto end_of_equal_groups_it = std::find_if_not(
        next(begin(available_groups)), end(available_groups),
        [g1](auto &g2) { return g1->free_capacity() == g2->free_capacity(); });

    return pick_random(begin(available_groups), end_of_equal_groups_it);
  }
  return fallback_policy();
}
//----------------------------------------------------------------------
std::optional<Group *> LowestFreeCapacity::find_next_group(const Load &load)
{
  auto available_groups = get_available_groups(load, group_->layer_);

  if (!available_groups.empty()) {
    sort(begin(available_groups), end(available_groups),
         [](const auto &group_ptr1, const auto &group_ptr2) {
           return group_ptr1->free_capacity() < group_ptr2->free_capacity();
         });

    auto &g1 = available_groups.front();
    auto end_of_equal_groups_it = std::find_if_not(
        next(begin(available_groups)), end(available_groups),
        [g1](auto &g2) { return g1->free_capacity() == g2->free_capacity(); });
    return pick_random(begin(available_groups), end_of_equal_groups_it);
  }
  return fallback_policy();
}

} // namespace overflow_policy
