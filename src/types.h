
#pragma once

#include <cstdint>
#include <string>
#include <experimental/memory>

using Time = long double;
using Duration = long double;
using Size = int64_t;
using Intensity = double;
using Uuid = uint64_t;
using Name = std::string;


// TODO(PW): when possible: remove this
using std::experimental::make_observer;
using std::experimental::observer_ptr;
