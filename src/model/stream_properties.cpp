
#include "stream_properties.h"

#include "logger.h"

#include <boost/variant/variant.hpp>

namespace Model
{
namespace
{
struct get_intensity_visitor : boost::static_visitor<Intensity> {
  auto operator()(const TrafficClass &tc) const
  {
    return tc.source_intensity / tc.serve_intensity;
  }
  auto operator()(const OverflowingRequestStream &rs) const
  {
    return rs.mean / rs.peakness;
  }
};

struct get_tc_visitor : boost::static_visitor<const TrafficClass &> {
  auto &operator()(const TrafficClass &tc) const { return tc; }
  auto &operator()(const OverflowingRequestStream &rs) const { return rs.tc; }
};

struct get_mean_visitor : boost::static_visitor<MeanIntensity> {
  auto operator()(const TrafficClass &tc) const
  {
    return MeanIntensity{get_intensity_visitor()(tc)};
  }
  auto operator()(const OverflowingRequestStream &rs) const { return rs.mean; }
};

struct get_variance_visitor : boost::static_visitor<Variance> {
  auto operator()(const TrafficClass &tc) const
  {
    return Variance{get_intensity_visitor()(tc)};
  }
  auto operator()(const OverflowingRequestStream &rs) const { return rs.variance; }
};
} // namespace

//----------------------------------------------------------------------

const TrafficClass &
get_tc(const StreamProperties &v)
{
  return boost::apply_visitor(get_tc_visitor(), v);
}

Intensity
get_intensity(const StreamProperties &v)
{
  return boost::apply_visitor(get_intensity_visitor(), v);
}

Variance
get_variance(const StreamProperties &v)
{
  return boost::apply_visitor(get_variance_visitor(), v);
}

MeanIntensity
get_mean(const StreamProperties &v)
{
  return boost::apply_visitor(get_mean_visitor(), v);
}

//----------------------------------------------------------------------
OverflowingRequestStream &
OverflowingRequestStream::operator+=(const RequestStream &rs)
{
  ASSERT(
      tc.id == TrafficClassId{0} || tc.id == rs.tc.id,
      "Adding statistics of two different traffic classes ({}!={})",
      tc.id,
      rs.tc.id);
  mean += rs.mean;
  variance += rs.variance;
  peakness = variance / mean;
  tc = rs.tc;

  return *this;
}

OverflowingRequestStream::OverflowingRequestStream(const RequestStream &rs)
  : tc(rs.tc), mean(rs.mean), variance(rs.variance), peakness(variance / mean)
{
}

OverflowingRequestStream
operator+(OverflowingRequestStream overflowing_rs, const RequestStream &rs)
{
  overflowing_rs += rs;
  return overflowing_rs;
}
//----------------------------------------------------------------------

void
format_arg(
    fmt::BasicFormatter<char> &f, const char *& /* format_str */, const RequestStream &rs)
{
  f.writer().write(
      "[RequestStream] {} Blocking probability {}, V={}, R={}, sigma^2={}, Z={}, Y={}",
      rs.tc,
      rs.blocking_probability,
      rs.fictional_capacity,
      rs.mean,
      rs.variance,
      rs.peakness,
      rs.mean_request_number);
}

//----------------------------------------------------------------------
void
format_arg(
    fmt::BasicFormatter<char> &f,
    const char *& /* format_str */,
    const OverflowingRequestStream &rs)
{
  f.writer().write(
      "[OverflowingRequestStream] {} R={}, sigma^2={}, Z={}",
      rs.tc,
      rs.mean,
      rs.variance,
      rs.peakness);
}

} // namespace Model

//----------------------------------------------------------------------

namespace boost
{
void
format_arg(
    fmt::BasicFormatter<char> &f,
    const char *&format_str,
    const Model::StreamProperties &stream_properties)
{
  boost::apply_visitor(
      [&](const auto &value) { format_arg(f, format_str, value); }, stream_properties);
}
} // namespace boost
