#include "stats.h"


//----------------------------------------------------------------------

LoadStats operator+(const LoadStats &s1, const LoadStats &s2)
{
  return {s1.count + s2.count, s1.size + s2.size};
}

LoadStats &operator+=(LoadStats &s1, const LoadStats &s2)
{
  s1.count += s2.count;
  s1.size += s2.size;
  return s1;
}

LostServedStats operator+(const LostServedStats &s1, const LostServedStats &s2)
{
  return {s1.lost + s2.lost, s1.served + s2.served, s1.forwarded + s2.forwarded};
}

LostServedStats &operator+=(LostServedStats &s1, const LostServedStats &s2)
{
  s1.lost += s2.lost;
  s1.served += s2.served;
  s1.forwarded += s2.forwarded;
  return s1;
}

//----------------------------------------------------------------------

bool BlockStats::try_block(const Time &time)
{
  if (!is_blocked) {
    is_blocked = true;
    start_of_block = time;
    return true;
  }
  return false;
}

bool BlockStats::try_unblock(const Time &time)
{
  if (is_blocked) {
    is_blocked = false;
    block_time += time - start_of_block;
    return true;
  }
  return false;
}
