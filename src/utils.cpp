
#include "utils.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

nlohmann::json
concatenate(nlohmann::json target, const nlohmann::json &patch)
{
  if (patch.is_object())
  {
    if (not target.is_object())
    {
      target = {};
    }
    for (auto it = patch.begin(); it != patch.end(); ++it)
    {
      if (target.is_null() && it.value().is_null())
      {
      }
      else if (it.value().is_null())
      {
        target.erase(it.key());
      }
      else
      {
        target[it.key()] = concatenate(target[it.key()], it.value());
      }
    }
  }
  else if (patch.is_array() && target.is_array())
  {
    target.insert(target.end(), patch.begin(), patch.end());
  }
  else
  {
    target = patch;
  }
  return target;
}

std::string
join(const std::vector<std::string> &strings, const std::string &separator)
{
  return fmt::format("{}", fmt::join(strings, separator));
}
