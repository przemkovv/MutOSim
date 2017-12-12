#include "events.h"

bool operator<(const Event &e1, const Event &e2)
{
  return e1.time < e2.time;
}
