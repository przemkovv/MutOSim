
#pragma once

#include "types.h"

#include <nlohmann/json.hpp>


template <typename T, typename UT>
void from_json(const nlohmann::json &j, ts::strong_typedef<T, UT> &value)
{
  value = T(j.get<UT>());
}
template <typename T, typename UT>
void to_json(nlohmann::json &j, const ts::strong_typedef<T, UT> &value)
{
  j = ts::get(value);
}
