
#pragma once

#include "types/types.h"

#include <functional>
#include <type_safe/strong_typedef.hpp>

namespace std {
/// Hash specialization for [GroupName].
template <>
struct hash<GroupName>
{
  std::size_t operator()(const GroupName &i) const noexcept
  {
    using T = ts::underlying_type<GroupName>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [SourceName].
template <>
struct hash<SourceName>
{
  std::size_t operator()(const SourceName &i) const noexcept
  {
    using T = ts::underlying_type<SourceName>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [TrafficClassId].
template <>
struct hash<TrafficClassId>
{
  std::size_t operator()(const TrafficClassId &i) const noexcept
  {
    using T = ts::underlying_type<TrafficClassId>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [LoadId].
template <>
struct hash<LoadId>
{
  std::size_t operator()(const LoadId &i) const noexcept
  {
    using T = ts::underlying_type<LoadId>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [SourceId].
template <>
struct hash<SourceId>
{
  std::size_t operator()(const SourceId &i) const noexcept
  {
    using T = ts::underlying_type<SourceId>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [Size].
template <>
struct hash<Size>
{
  std::size_t operator()(const Size &i) const noexcept
  {
    using T = ts::underlying_type<Size>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [Capacity].
template <>
struct hash<Model::Capacity>
{
  std::size_t operator()(const Model::Capacity &i) const noexcept
  {
    using T = ts::underlying_type<Model::Capacity>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
/// Hash specialization for [Capacity].
/*
template <>
struct hash<std::enable_if_t<
    !std::is_same_v<Model::Capacity, Simulation::Capacity>,
    Simulation::Capacity>>
{
  std::size_t operator()(const Simulation::Capacity &i) const noexcept
  {
    using T = ts::underlying_type<Simulation::Capacity>;
    return std::hash<T>()(static_cast<T>(i));
  }
};
*/
} // namespace std
