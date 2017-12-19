
#pragma once

#include <boost/multiprecision/mpfr.hpp>
#include <cstdint>
#include <experimental/memory>
#include <fmt/ostream.h>

#include <string>

using Time = long double;
using Duration = long double;
// using Time = boost::multiprecision::static_mpfr_float_50;
// using Duration = boost::multiprecision::static_mpfr_float_50;
using Size = int64_t;
using Intensity = double;
using Uuid = uint64_t;
using Name = std::string;

// TODO(PW): when possible: remove this
using std::experimental::make_observer;
using std::experimental::observer_ptr;

template <typename T>
void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const boost::multiprecision::number<T> &number)
{
  f.writer().write("{}", number.str());
}
