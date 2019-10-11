
#pragma once
#include "precision.h"

#include <range/v3/utility/iterator_concepts.hpp>

namespace ranges {

template <typename Prec>
struct incrementable_traits<TypesPrecision::Count_<Prec>>
{
  using difference_type = ::ts::underlying_type<TypesPrecision::Count_<Prec>>;
};

template <typename Prec, typename UseFloat>
struct incrementable_traits<TypesPrecision::Capacity_<Prec, UseFloat>>
{
  using difference_type =
      ::ts::underlying_type<TypesPrecision::Capacity_<Prec, UseFloat>>;
};

} // namespace ranges
