
#include "factory.h"

#include "logger.h"
#include "overflow_policy.h"

namespace Simulation
{
namespace overflow_policy
{
std::unique_ptr<overflow_policy::OverflowPolicy>
make_overflow_policy(std::optional<std::string_view> name, Group &group)
{
  using namespace overflow_policy;
  using namespace std::literals;
  if (name) {
    if (name == "random_available"sv) {
      return std::make_unique<RandomAvailable>(group);
    }
    if (name == "first_available"sv) {
      return std::make_unique<AlwaysFirst>(group);
    }
    if (name == "always_first"sv) {
      return std::make_unique<AlwaysFirst>(group);
    }
    if (name == "no_overflow"sv) {
      return std::make_unique<NoOverflow>(group);
    }
    if (name == "highest_free_capacity"sv) {
      return std::make_unique<HighestFreeCapacity>(group);
    }
    if (name == "lowest_free_capacity"sv) {
      return std::make_unique<LowestFreeCapacity>(group);
    }
    if (name == "default"sv) {
      return std::make_unique<Default>(group);
    }
    print("[Main]: Don't recognize '{}' overflow policy name. Using default.", *name);
  }
  return std::make_unique<Default>(group);
}

} // namespace overflow_policy
} // namespace Simulation
