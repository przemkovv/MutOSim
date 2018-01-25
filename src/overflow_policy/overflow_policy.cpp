
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
  std::vector<observer_ptr<Group>> available_groups;
  available_groups.reserve(group_->next_groups_.size());
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [&](const auto &group) {
                 return group->can_serve(load.size) && !contains(load.path, group) &&
                        count_same_layer_groups(load.path, group->layer_) <=
                            overflows_per_layer;
               });
  sort(begin(available_groups), end(available_groups),
       [](const auto &group_ptr1, const auto &group_ptr2) {
         if (group_ptr1->layer_ == group_ptr2->layer_) {
           return group_ptr1->free_capacity() < group_ptr2->free_capacity();
         } else {
           return group_ptr1->layer_ < group_ptr2->layer_;
         }
       });
  std::vector<observer_ptr<Group>> available_groups_same_layer;
  std::copy_if(begin(available_groups), end(available_groups),
               back_inserter(available_groups_same_layer),
               [&layer = available_groups.front()->layer_](const auto &group) {
                 return group->layer_ == layer;
               });
  if (!available_groups_same_layer.empty()) {
    return get_random_element(available_groups_same_layer, world_->get_random_engine());
  }
  return {};
}
//----------------------------------------------------------------------
std::optional<observer_ptr<Group>> HighestFreeCapacity::find_next_group(const Load &load)
{
  std::vector<observer_ptr<Group>> available_groups;
  
  available_groups.reserve(group_->next_groups_.size());
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [&](const auto &group) {
                 return group->can_serve(load.size) && !contains(load.path, group) &&
                        count_same_layer_groups(load.path, group->layer_) <=
                            overflows_per_layer;
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
    return available_groups.front();
  }
  return {};
}
//----------------------------------------------------------------------
std::optional<observer_ptr<Group>> LowestFreeCapacity::find_next_group(const Load &load)
{
  std::vector<observer_ptr<Group>> available_groups;

  available_groups.reserve(group_->next_groups_.size());
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [&](const auto &group) {
                 return group->can_serve(load.size) && !contains(load.path, group) &&
                        count_same_layer_groups(load.path, group->layer_) <=
                            overflows_per_layer;
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
    return available_groups.front();
  }
  return {};
}

} // namespace overflow_policy
