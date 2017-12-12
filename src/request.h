
#pragma once
#include "types.h"


template <typename T>
RequestId to_id(T value)
{
  return static_cast<RequestId>(value);
}

struct Request {
  RequestId id;
  int64_t count;
};
