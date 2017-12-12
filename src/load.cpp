
#include "load.h"

bool operator<(const Load &l1, const Load &l2)
{
  return l1.end_time < l2.end_time;
}
