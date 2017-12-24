
#include "load.h"
#include <fmt/ostream.h>

void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Load &load)
{
  if (load.end_time < Time{0}) {
    f.writer().write("[Load: id={}, size={} st={}, p_by={}]", load.id, load.size,
                     load.send_time, load.produced_by != nullptr);
  } else {
    f.writer().write("[Load: id={}, size={}, st={}, dt={}, p_by={}]", load.id, load.size,
                     load.send_time, load.end_time - load.send_time, load.produced_by != nullptr);
  }
}
