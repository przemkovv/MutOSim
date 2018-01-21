
#include "load.h"
#include "source_stream/source_stream.h"
#include "group.h"

#include <fmt/ostream.h>


void format_arg(fmt::BasicFormatter<char> &f,
                const char *& /* format_str */,
                const Load &load)
{
  if (load.end_time < Time{0}) {
    f.writer().write("[Load: id={}, size={} st={}]", load.id, load.size,
                     load.send_time);
  } else {
    f.writer().write("[Load: id={}, size={}, st={}, dt={}]", load.id, load.size,
                     load.send_time, load.end_time - load.send_time);
  }
}
