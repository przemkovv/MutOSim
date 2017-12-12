
#include "load.h"

bool operator<(const Load &l1, const Load &l2)
{
  return l1.end_time < l2.end_time;
}

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Load &load)
{
  if (load.end_time < 0) {
    f.writer().write("[Load: id={}, size={} st={}]", load.id, load.size,
                     load.send_time);
  } else {
    f.writer().write("[Load: id={}, size={}, st={}, dt={}]", load.id, load.size,
                     load.send_time, load.end_time - load.send_time);
  }
}
