
#include "traffic_class.h"

void format_arg(fmt::BasicFormatter<char> &f,
                const char *&/* format_str */,
                const TrafficClass &tc)
{
  f.writer().write("[TC: id={:>2}, l={:<8} u={}, size={}]", tc.id,
                   tc.source_intensity, tc.serve_intensity, tc.size);
}
