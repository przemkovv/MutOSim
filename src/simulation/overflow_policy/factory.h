
#pragma once

#include <gsl/gsl>
#include <optional>
#include <string_view>

struct Group;

namespace overflow_policy
{
class OverflowPolicy;

std::unique_ptr<OverflowPolicy> make_overflow_policy(std::optional<std::string_view> name,
                                                     Group &group);

} // namespace overflow_policy
