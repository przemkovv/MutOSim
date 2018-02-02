
#include "overflow_policy.h"

#include "group.h"
#include "load.h"
#include "source_stream/source_stream.h"
#include "utils.h"

#include <type_traits>

namespace overflow_policy
{
OverflowPolicy::OverflowPolicy(gsl::not_null<Group *> group)
  : group_(make_observer(group.get()))
{
}

void OverflowPolicy::set_world(gsl::not_null<World *> world)
{
  world_ = make_observer(world.get());
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

std::optional<observer_ptr<Group>> OverflowPolicy::fallback_policy()
{
  std::vector<observer_ptr<Group>> available_groups;
  available_groups.reserve(group_->next_groups_.size());
  auto current_layer = group_->layer_;
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [current_layer](const auto &group) {
                 return group->layer_ > current_layer;
               });
  if (!available_groups.empty()) {
    return get_random_element(begin(available_groups), end(available_groups),
                              world_->get_random_engine());
  }
  return {};
}

//----------------------------------------------------------------------
std::optional<observer_ptr<Group>> NoOverflow::find_next_group(const Load & /* load */)
{
  return {};
}

//----------------------------------------------------------------------
std::optional<observer_ptr<Group>> FirstAvailable::find_next_group(const Load &load)
{
  for (const auto &next_group : group_->next_groups_) {
    if (find(begin(load.path), end(load.path), next_group) == end(load.path)) {
      auto is_served = next_group->can_serve(load.size);
      if (is_served) {
        return next_group;
      }
    }
  }
  return {};
}

//----------------------------------------------------------------------
std::optional<observer_ptr<Group>> AlwaysFirst::find_next_group(const Load &load)
{
  for (const auto &next_group : group_->next_groups_) {
    if (find(begin(load.path), end(load.path), next_group) == end(load.path)) {
      return next_group;
    }
  }
  return {};
}

//----------------------------------------------------------------------
std::optional<observer_ptr<Group>> OverflowPolicy::find_next_group(const Load &load)
{
  for (const auto &next_group : group_->next_groups_) {
    if (find(begin(load.path), end(load.path), next_group) == end(load.path)) {
      return next_group;
    }
  }
  return {};
}

//----------------------------------------------------------------------
std::optional<observer_ptr<Group>> RandomAvailable::find_next_group(const Load &load)
{
  const auto layers_usage = count_layers_usage(load.path);

  std::vector<observer_ptr<Group>> available_groups;
  available_groups.reserve(group_->next_groups_.size());
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [&](const auto &group) {
                 return layers_usage[group->layer_] < overflows_per_layer &&
                        group->can_serve(load.size) && !contains(load.path, group);
               });

  if (!available_groups.empty()) {
    if (available_groups.size() == 1) {
      return available_groups.front();
    }
    sort(begin(available_groups), end(available_groups),
         [](const auto &group_ptr1, const auto &group_ptr2) {
           return group_ptr1->layer_ < group_ptr2->layer_;
         });

    auto layer = available_groups.front()->layer_;
    auto end_of_layer_it =
        std::find_if_not(next(begin(available_groups)), end(available_groups),
                         [=](const auto &group) { return group->layer_ == layer; });
    return get_random_element(begin(available_groups), end_of_layer_it,
                              world_->get_random_engine());
  }
  return fallback_policy();
}
//----------------------------------------------------------------------
std::optional<observer_ptr<Group>> HighestFreeCapacity::find_next_group(const Load &load)
{
  const auto layers_usage = count_layers_usage(load.path);
  std::vector<observer_ptr<Group>> available_groups;

  available_groups.reserve(group_->next_groups_.size());
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [&](const auto &group) {
                 return layers_usage[group->layer_] < overflows_per_layer &&
                        group->can_serve(load.size) && !contains(load.path, group);
               });

  if (!available_groups.empty()) {
    sort(begin(available_groups), end(available_groups),
         [](const auto &group_ptr1, const auto &group_ptr2) {
           if (group_ptr1->layer_ == group_ptr2->layer_) {
             return group_ptr1->free_capacity() > group_ptr2->free_capacity();
           } else {
             return group_ptr1->layer_ < group_ptr2->layer_;
           }
         });

    auto &g1 = available_groups.front();
    auto end_of_equal_groups_it = std::find_if_not(
        next(begin(available_groups)), end(available_groups), [g1](auto &g2) {
          return g1->free_capacity() == g2->free_capacity() && g1->layer_ == g2->layer_;
        });
    return get_random_element(begin(available_groups), end_of_equal_groups_it,
                              world_->get_random_engine());
  }
  return fallback_policy();
}
//----------------------------------------------------------------------
std::optional<observer_ptr<Group>> LowestFreeCapacity::find_next_group(const Load &load)
{
  const auto layers_usage = count_layers_usage(load.path);
  std::vector<observer_ptr<Group>> available_groups;

  available_groups.reserve(group_->next_groups_.size());
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [&](const auto &group) {
                 return layers_usage[group->layer_] < overflows_per_layer &&
                        group->can_serve(load.size) && !contains(load.path, group);
               });

  if (!available_groups.empty()) {
    sort(begin(available_groups), end(available_groups),
         [](const auto &group_ptr1, const auto &group_ptr2) {
           if (group_ptr1->layer_ == group_ptr2->layer_) {
             return group_ptr1->free_capacity() < group_ptr2->free_capacity();
           } else {
             return group_ptr1->layer_ < group_ptr2->layer_;
           }
         });

    auto &g1 = available_groups.front();
    auto end_of_equal_groups_it = std::find_if_not(
        next(begin(available_groups)), end(available_groups), [g1](auto &g2) {
          return g1->free_capacity() == g2->free_capacity() && g1->layer_ == g2->layer_;
        });
    return get_random_element(begin(available_groups), end_of_equal_groups_it,
                              world_->get_random_engine());
  }
  return fallback_policy();
}

} // namespace overflow_policy
