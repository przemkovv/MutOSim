
#pragma once
#include "precision.h"

#include <range/v3/utility/iterator_concepts.hpp>

namespace ranges {
inline namespace v3 {

template <typename Prec>
struct difference_type<TypesPrecision::Count_<Prec>>
{
  using type = ::ts::underlying_type<TypesPrecision::Count_<Prec>>;
};

template <typename Prec, typename UseFloat>
struct difference_type<TypesPrecision::Capacity_<Prec, UseFloat>>
{
  using type = ::ts::underlying_type<TypesPrecision::Capacity_<Prec, UseFloat>>;
};


} // namespace v3
} // namespace ranges
