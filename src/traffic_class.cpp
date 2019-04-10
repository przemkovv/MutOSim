
#include "traffic_class.h"

bool
operator==(const TrafficClass &tc1, const TrafficClass &tc2)
{
  return tc1.id == tc2.id && tc1.source_intensity == tc2.source_intensity
         && tc1.serve_intensity == tc2.serve_intensity && tc1.size == tc2.size
         && tc1.max_path_length == tc2.max_path_length;
}
