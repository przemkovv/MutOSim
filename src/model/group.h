
#pragma once
#include "stream_properties.h"
#include "types.h"

#include <map>
#include <range/v3/algorithm/for_each.hpp>
#include <vector>

namespace Model
{
struct Group {
private:
  std::map<TrafficClassId, IncomingRequestStream> in_request_streams_;
  const Capacity V_;

  mutable std::vector<OutgoingRequestStream> out_request_streams_;
  mutable bool need_recalculate_ = true;

public:
  const std::vector<OutgoingRequestStream> &get_outgoing_request_streams() const;

  Group(Capacity V);

  template <typename RequestStream>
  void add_incoming_request_stream(const RequestStream &request_stream);
  template <typename RequestStream>
  void add_incoming_request_stream(const std::vector<RequestStream> &request_streams);
};

template <typename RequestStream>
void
Group::add_incoming_request_stream(const RequestStream &in_request_stream)
{
  static_assert(
      std::is_same_v<RequestStream, IncomingRequestStream> ||
          std::is_same_v<RequestStream, OutgoingRequestStream>,
      "Only IncomingRequestStream and OutgoingRequestStream request stream types are "
      "supported.");

  // Formulas 3.17 and 3.18
  if (auto [rs_it, inserted] =
          in_request_streams_.try_emplace(in_request_stream.tc.id, in_request_stream);
      !inserted) {
    rs_it->second += in_request_stream;
  }
  need_recalculate_ = true;
}

template <typename RequestStream>
void
Group::add_incoming_request_stream(const std::vector<RequestStream> &in_request_streams)
{
  ranges::for_each(in_request_streams, [this](const auto &rs) {
    this->add_incoming_request_stream(rs);
  });
}
} // namespace Model
