
#include "stream_properties.h"

#include "logger.h"
#include "types/types_format.h"

namespace Model
{
OutgoingRequestStream::OutgoingRequestStream(
    const TrafficClass &tc_,
    const Probability &blocking_probability_,
    const Intensity &intensity_)
  : tc(tc_),
    blocking_probability(blocking_probability_),
    intensity(intensity_),
    mean(intensity * blocking_probability),
    mean_request_number(intensity * blocking_probability.opposite())
{
}
//----------------------------------------------------------------------
IncomingRequestStream &
IncomingRequestStream::operator+=(const TrafficClass &source_tc)
{
  ASSERT(
      tc.id == TrafficClassId{0} || tc.id == source_tc.id,
      "Adding statistics of two different traffic classes ({}!={})",
      tc.id,
      source_tc.id);
  *this += IncomingRequestStream(source_tc);
  return *this;
}

IncomingRequestStream &
IncomingRequestStream::operator+=(const IncomingRequestStream &rs)
{
  ASSERT(
      tc.id == TrafficClassId{0} || tc.id == rs.tc.id,
      "Adding statistics of two different traffic classes ({}!={})",
      tc.id,
      rs.tc.id);
  mean += rs.mean;
  variance += rs.variance;
  peakedness = variance / mean;
  intensity = mean / peakedness;
  tc = rs.tc;

  return *this;
}
IncomingRequestStream &
IncomingRequestStream::operator+=(const OutgoingRequestStream &rs)
{
  ASSERT(
      tc.id == TrafficClassId{0} || tc.id == rs.tc.id,
      "Adding statistics of two different traffic classes ({}!={})",
      tc.id,
      rs.tc.id);
  *this += IncomingRequestStream(rs);
  return *this;
}

IncomingRequestStream::IncomingRequestStream(const IncomingRequestStream &rs)
  : tc(rs.tc),
    mean(rs.mean),
    variance(rs.variance),
    peakedness(variance / mean),
    intensity(mean / peakedness)
{
}
IncomingRequestStream::IncomingRequestStream(const OutgoingRequestStream &rs)
  : tc(rs.tc),
    mean(rs.mean),
    variance(rs.variance),
    peakedness(variance / mean),
    intensity(mean / peakedness)
{
}
IncomingRequestStream::IncomingRequestStream(const TrafficClass &source_tc)
  : tc(source_tc),
    mean(source_tc.source_intensity / source_tc.serve_intensity),
    variance(source_tc.source_intensity / source_tc.serve_intensity),
    peakedness(1.0L),
    intensity(source_tc.source_intensity / source_tc.serve_intensity)
{
}

} // namespace Model

//----------------------------------------------------------------------

