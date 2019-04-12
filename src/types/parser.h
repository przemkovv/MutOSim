
#pragma once

#include "types/types.h"

#include <nlohmann/json.hpp>

namespace type_safe {
template <typename T, typename UT>
void
from_json(const nlohmann::json &j, ts::strong_typedef<T, UT> &value)
{
  value = T(j.get<UT>());
}
template <typename T, typename UT>
void
to_json(nlohmann::json &j, const ts::strong_typedef<T, UT> &value)
{
  j = ts::get(value);
}

} // namespace type_safe

inline void
from_json(const nlohmann::json &j, GroupName &value)
{
  value = GroupName{j.get<GroupName::value_type>()};
}
inline void
to_json(nlohmann::json &j, const GroupName &value)
{
  j = ts::get(value);
}
