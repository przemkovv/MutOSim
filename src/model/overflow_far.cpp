
#include "overflow_far.h"

namespace Model
{
float KaufmanRoberts(std::vector<RequestStream> &request_streams, Capacity V, Capacity n)
{
  float value = 0;
  for (const auto &request_stream : request_streams) {
    if (n >= request_stream.tc.size) {
      value += get(request_stream.intensity) * get(request_stream.tc.size) *
               KaufmanRoberts(request_streams, V, n - request_stream.tc.size);
    }
  }
  return value / get(n);
}

} // namespace Model
