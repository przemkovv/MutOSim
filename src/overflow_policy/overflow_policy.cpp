
#include "overflow_policy.h"

#include "group.h"
#include "load.h"
#include "source_stream/source_stream.h"

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
  auto contains = [](const auto &container, const auto &value) {
    return find(begin(container), end(container), value) != end(container);
  };

  auto get_random_element = [](const auto &container, auto &random_engine) {
    std::remove_cv_t<std::remove_reference_t<decltype(*begin(container))>> elem;
    std::sample(begin(container), end(container), &elem, 1, random_engine);
    return elem;
  };

  std::vector<observer_ptr<Group>> available_groups;
  available_groups.reserve(group_->next_groups_.size());
  std::copy_if(begin(group_->next_groups_), end(group_->next_groups_),
               back_inserter(available_groups), [&](const auto &group) {
                 return group->can_serve(load.size) && !contains(load.path, group);
               });
  if (!available_groups.empty()) {
    return get_random_element(available_groups, world_->get_random_engine());
  }
  return {};
}

} // namespace overflow_policy
