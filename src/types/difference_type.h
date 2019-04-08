
#pragma once
#include "precision.h"

#include <range/v3/utility/iterator_concepts.hpp>

namespace ranges {
inline namespace v3 {
// template <>
template <typename Prec>
struct difference_type<TypesPrecision::Count<Prec>>
{
  using type = ::ts::underlying_type<TypesPrecision::Count<Prec>>;
};
// template <>
template <typename Prec, typename UseFloat>
struct difference_type<TypesPrecision::Capacity<Prec, UseFloat>>
{
  using type = ::ts::underlying_type<TypesPrecision::Capacity<Prec, UseFloat>>;
};
} // namespace v3
} // namespace ranges
