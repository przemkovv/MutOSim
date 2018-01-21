
#pragma once

#include <gsl/gsl>
#include <string_view>
#include <optional>

struct Group;

namespace overflow_policy
{
class OverflowPolicy;

std::unique_ptr<OverflowPolicy> make_overflow_policy(std::optional<std::string_view> name,
                                                     gsl::not_null<Group *> group);

} // namespace overflow_policy
