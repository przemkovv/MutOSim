
#include "request_stream.h"

Request RequestStream::get(Time time)
{
  Request r{to_id(time), 1};
  return r;
}
