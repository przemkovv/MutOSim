
#pragma once

#include "common.h"
#include "precision.h"

#include <string_view>

namespace ts = type_safe;
namespace Model {
using Intensity = TypesPrecision::Intensity<highp, use_float_tag>;
using IntensityFactor = TypesPrecision::IntensityFactor<highp>;
using Ratio = TypesPrecision::Ratio<highp>;
using Capacity = TypesPrecision::Capacity<mediump, use_int_tag>;
using Count = TypesPrecision::Count<mediump>;

} // namespace Model

namespace Simulation {
using Intensity = TypesPrecision::Intensity<mediump, use_float_tag>;
using IntensityFactor = TypesPrecision::IntensityFactor<mediump>;
using Ratio = TypesPrecision::Ratio<mediump>;
using Capacity = TypesPrecision::Capacity<mediump, use_int_tag>;
using Count = TypesPrecision::Count<mediump>;
} // namespace Simulation

using CapacityF = TypesPrecision::Capacity<highp, use_float_tag>;
using Weight = TypesPrecision::Weight<mediump, use_int_tag>;
using WeightF = TypesPrecision::Weight<highp, use_float_tag>;
using Size = TypesPrecision::Size<mediump, use_int_tag>;
using SizeF = TypesPrecision::Size<highp, use_float_tag>;

using SizeRescale = TypesPrecision::SizeRescale<highp>;
using Time = TypesPrecision::Time<mediump>;
using Duration = TypesPrecision::Duration<mediump>;
using Probability = TypesPrecision::Probability<highp>;
using MeanIntensity = TypesPrecision::MeanIntensity<highp>;
using Peakedness = TypesPrecision::Peakedness<highp>;
using Variance = TypesPrecision::Variance<highp>;
using MeanRequestNumber = TypesPrecision::MeanRequestNumber<highp>;

struct GroupName : ts::strong_typedef<GroupName, name_t>,
                   ts::strong_typedef_op::equality_comparison<GroupName>,
                   ts::strong_typedef_op::relational_comparison<GroupName>,
                   ts::strong_typedef_op::output_operator<GroupName>
{
  using strong_typedef::strong_typedef;
  GroupName operator+(std::string_view str)
  {
    return GroupName(ts::get(*this) + name_t(str));
  }
};

struct SourceName : ts::strong_typedef<SourceName, name_t>,
                    ts::strong_typedef_op::equality_comparison<SourceName>,
                    ts::strong_typedef_op::relational_comparison<SourceName>,
                    ts::strong_typedef_op::output_operator<SourceName>
{
  using strong_typedef::strong_typedef;
};

struct GroupId : ts::strong_typedef<GroupId, uuid_t>,
                 ts::strong_typedef_op::equality_comparison<GroupId>,
                 ts::strong_typedef_op::output_operator<GroupId>
{
  using strong_typedef::strong_typedef;
};
struct SourceId : ts::strong_typedef<SourceId, uuid_t>,
                  ts::strong_typedef_op::equality_comparison<SourceId>,
                  ts::strong_typedef_op::output_operator<SourceId>
{
  using strong_typedef::strong_typedef;
};
struct LoadId : ts::strong_typedef<LoadId, uuid_t>,
                ts::strong_typedef_op::equality_comparison<LoadId>,
                ts::strong_typedef_op::output_operator<LoadId>
{
  using strong_typedef::strong_typedef;
};

struct TrafficClassId
  : ts::strong_typedef<TrafficClassId, uuid_t>,
    ts::strong_typedef_op::equality_comparison<TrafficClassId>,
    ts::strong_typedef_op::relational_comparison<TrafficClassId>,
    ts::strong_typedef_op::output_operator<TrafficClassId>
{
  using strong_typedef::strong_typedef;
};
