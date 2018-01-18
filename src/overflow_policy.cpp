
#include "overflow_policy.h"

#include "group.h"
#include "load.h"

namespace overflow_policy
{
OverflowPolicy::OverflowPolicy(gsl::not_null<Group *> group)
  : group_(make_observer(group.get()))
{
}

std::optional<observer_ptr<Group>> AlwaysFirst::find_next_group(const Load &load)
{
  for (const auto &next_group : group_->next_groups_) {
    if (find(begin(load.path), end(load.path), next_group) == end(load.path)) {
      return next_group;
      // auto is_served = next_group->can_serve(load.size);
      // if (is_served) {
        // return next_group;
      // }
    }
  }
  return {};
}
std::optional<observer_ptr<Group>> OverflowPolicy::find_next_group(const Load &load)
{
  for (const auto &next_group : group_->next_groups_) {
    if (find(begin(load.path), end(load.path), next_group) == end(load.path)) {
      return next_group;
      // auto is_served = next_group->can_serve(load.size);
      // if (is_served) {
        // return next_group;
      // }
    }
  }
  return {};
}

} // namespace overflow_policy
