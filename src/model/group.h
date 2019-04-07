
#pragma once

#include "common.h"
#include "resource.h"
#include "stream_properties.h"
#include "types/types.h"

#include <map>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/numeric.hpp>
#include <vector>

namespace Model {
struct Group
{
private:
  std::map<TrafficClassId, IncomingRequestStream> in_request_streams_{};
  const Resource<>                                  resource_;
  const Capacity                                  total_V_ = resource_.V();

  mutable OutgoingRequestStreams out_request_streams_{};
  mutable bool                   need_recalculate_ = true;
  const KaufmanRobertsVariant    kr_variant_;

  std::vector<GroupName> next_groups_names_{};

public:
  const OutgoingRequestStreams &get_outgoing_request_streams() const;

  Group(Capacity V, KaufmanRobertsVariant kr_variant);
  Group(std::vector<Capacity> V, KaufmanRobertsVariant kr_variant);
  Group(Resource<> resource, KaufmanRobertsVariant kr_variant);

  template <typename RequestStream>
  void add_incoming_request_stream(const RequestStream &request_stream);
  template <typename RequestStream>
  void add_incoming_request_streams(const std::vector<RequestStream> &request_streams);

  void add_next_group(GroupName group_name)
  {
    next_groups_names_.emplace_back(std::move(group_name));
  }
  const std::vector<GroupName> next_groups() { return next_groups_names_; }
};

template <typename RequestStream>
void
Group::add_incoming_request_stream(const RequestStream &in_rs)
{
  static_assert(
      std::is_same_v<
          RequestStream,
          IncomingRequestStream> || std::is_same_v<RequestStream, OutgoingRequestStream>,
      "Only IncomingRequestStream and OutgoingRequestStream request stream types are "
      "supported.");

  // Formulas 3.17 and 3.18
  if (auto [rs_it, inserted] = in_request_streams_.try_emplace(in_rs.tc.id, in_rs); !inserted)
  {
    rs_it->second += in_rs;
  }
  need_recalculate_ = true;
}

template <typename RequestStream>
void
Group::add_incoming_request_streams(const std::vector<RequestStream> &in_request_streams)
{
  ranges::for_each(in_request_streams, [this](const auto &rs) {
    this->add_incoming_request_stream(rs);
  });
}
} // namespace Model
