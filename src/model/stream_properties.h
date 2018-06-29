#pragma once

#include "traffic_class.h"
#include "types.h"

#include <vector>

namespace Model
{
struct OutgoingRequestStream {
  TrafficClass tc;
  Probability blocking_probability{0}; // [E_c]_{V_s}
  Intensity intensity{0};

  MeanIntensity mean{0};                    // R_{c,s}
  MeanRequestNumber mean_request_number{0}; // Y_{c,s}

  CapacityF fictitous_capacity{0};
  Variance variance{0};     // sigma^2_{c,s}
  Peakedness peakedness{0}; // Z_c

  OutgoingRequestStream() = default;
  OutgoingRequestStream(
      const TrafficClass &tc,
      const Probability &blocking_probability,
      const Intensity &intensity);
};

struct IncomingRequestStream {
  TrafficClass tc{};
  MeanIntensity mean{0};
  Variance variance{0};
  Peakedness peakedness{0};
  Intensity intensity{0};

  IncomingRequestStream &operator+=(const OutgoingRequestStream &rs);
  IncomingRequestStream &operator+=(const TrafficClass &source_tc);
  IncomingRequestStream &operator+=(const IncomingRequestStream &rs);
  IncomingRequestStream &operator=(const IncomingRequestStream &rs) = default;
  IncomingRequestStream(const OutgoingRequestStream &rs);
  IncomingRequestStream(const TrafficClass &source_tc);
  IncomingRequestStream(const IncomingRequestStream &rs);
  IncomingRequestStream() = default;
};

using OutgoingRequestStreams = std::vector<OutgoingRequestStream>;
using IncomingRequestStreams = std::vector<IncomingRequestStream>;

//----------------------------------------------------------------------

} // namespace Model

