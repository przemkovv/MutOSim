
#pragma once

#include <gsl/gsl>
#include <optional>
#include <string_view>

namespace Simulation {
struct Group;

class OverflowPolicy;

std::unique_ptr<OverflowPolicy>
make_overflow_policy(std::optional<std::string_view> name, Group &group);

} // namespace Simulation
