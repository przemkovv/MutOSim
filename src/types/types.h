
#pragma once

#include "common.h"
#include "precision.h"

#include <string_view>

namespace ts = type_safe;

namespace Model {

using precision = highp;

using Intensity = TypesPrecision::Intensity_<precision>;
using IntensityFactor = TypesPrecision::IntensityFactor_<precision>;
using Ratio = TypesPrecision::Ratio_<precision>;
using Capacity = TypesPrecision::Capacity_<precision, use_int_tag>;
using CapacityF = TypesPrecision::Capacity_<precision, use_float_tag>;
using Size = TypesPrecision::Size_<precision, use_int_tag>;
using SizeF = TypesPrecision::Size_<precision, use_float_tag>;
using Count = TypesPrecision::Count_<precision>;

using Probability = TypesPrecision::Probability_<precision>;
using MeanIntensity = TypesPrecision::MeanIntensity_<precision>;
using Peakedness = TypesPrecision::Peakedness_<precision>;
using Variance = TypesPrecision::Variance_<precision>;
using MeanRequestNumber = TypesPrecision::MeanRequestNumber_<precision>;

using WeightF = TypesPrecision::Weight_<precision, use_float_tag>;
using SizeRescale = TypesPrecision::SizeRescale_<precision>;

} // namespace Model

namespace Simulation {

using precision = mediump;

using Intensity = TypesPrecision::Intensity_<precision>;
using IntensityFactor = TypesPrecision::IntensityFactor_<precision>;
using Ratio = TypesPrecision::Ratio_<precision>;
using Capacity = TypesPrecision::Capacity_<precision, use_int_tag>;
using Count = TypesPrecision::Count_<precision>;
using Size = TypesPrecision::Size_<precision, use_int_tag>;
using SizeF = TypesPrecision::Size_<precision, use_float_tag>;

} // namespace Simulation

using Time = TypesPrecision::Time_<mediump>;
using Duration = TypesPrecision::Duration_<mediump>;
using Weight = TypesPrecision::Weight_<mediump, use_int_tag>;

struct GroupName : ts::strong_typedef<GroupName, name_t>,
                   ts::strong_typedef_op::equality_comparison<GroupName>,
                   ts::strong_typedef_op::relational_comparison<GroupName>,
                   ts::strong_typedef_op::output_operator<GroupName>
{
  using strong_typedef::strong_typedef;
  using value_type = ts::underlying_type<GroupName>;
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
  using value_type = ts::underlying_type<SourceName>;
};

struct GroupId : ts::strong_typedef<GroupId, uuid_t>,
                 ts::strong_typedef_op::equality_comparison<GroupId>,
                 ts::strong_typedef_op::output_operator<GroupId>
{
  using strong_typedef::strong_typedef;
  using value_type = ts::underlying_type<GroupId>;
};
struct SourceId : ts::strong_typedef<SourceId, uuid_t>,
                  ts::strong_typedef_op::equality_comparison<SourceId>,
                  ts::strong_typedef_op::output_operator<SourceId>
{
  using strong_typedef::strong_typedef;
  using value_type = ts::underlying_type<SourceId>;
};
struct LoadId : ts::strong_typedef<LoadId, uuid_t>,
                ts::strong_typedef_op::equality_comparison<LoadId>,
                ts::strong_typedef_op::output_operator<LoadId>
{
  using strong_typedef::strong_typedef;
  using value_type = ts::underlying_type<LoadId>;
};

struct TrafficClassId
  : ts::strong_typedef<TrafficClassId, uuid_t>,
    ts::strong_typedef_op::equality_comparison<TrafficClassId>,
    ts::strong_typedef_op::relational_comparison<TrafficClassId>,
    ts::strong_typedef_op::output_operator<TrafficClassId>
{
  using strong_typedef::strong_typedef;
  using value_type = ts::underlying_type<TrafficClassId>;
};

inline Model::Capacity
to_model(const Simulation::Capacity &capacity)
{
  return Model::Capacity{
      static_cast<Model::Capacity::value_type>(capacity.value())};
}

inline Model::Intensity
to_model(const Simulation::Intensity &intensity)
{
  return Model::Intensity{
      static_cast<Model::Intensity::value_type>(intensity.value())};
}
