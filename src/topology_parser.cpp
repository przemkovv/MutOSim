
#include "topology_parser.h"

#include <nlohmann/json.hpp>
#include <unordered_map>

using nlohmann::json;

void to_json(json &j, const Intensity &s);
void from_json(const json &j, Intensity &s);

void to_json(json &j, const Size &s);
void from_json(const json &j, Size &s);

void to_json(json &j, const Intensity &s)
{
  j = ts::get(s);
}
void from_json(const json &j, Intensity &s)
{
  s = Intensity(j.get<ts::underlying_type<Intensity>>());
}

void to_json(json &j, const Size &s)
{
  j = ts::get(s);
}
void from_json(const json &j, Size &s)
{
  s = Size(j.get<ts::underlying_type<Size>>());
}

namespace Config
{
void to_json(json &j, const SourceType &st);
void from_json(const json &j, SourceType &st);

void to_json(json &j, const TrafficClass &st);
void from_json(const json &j, TrafficClass &st);

void to_json(json &j, const Source &st);
void from_json(const json &j, Source &st);

void to_json(json &j, const Group &st);
void from_json(const json &j, Group &st);
void to_json(json &j, const SourceType &st)
{
  j = [=]() {
    switch (st) {
    case SourceType::Poisson:
      return "poisson";
    case SourceType::Engset:
      return "engset";
    case SourceType::Pascal:
      return "pascal";
    }
  }();
}

void from_json(const json &j, SourceType &st)
{
  const auto str = j;
  static const std::unordered_map<std::string, SourceType> m = {
      {"poisson", SourceType::Poisson},
      {"engset", SourceType::Engset},
      {"pascal", SourceType::Pascal},
  };

  const auto it = m.find(str);
  st = it->second;
}

void to_json(json &j, const TrafficClass &st)
{
  j = {{"micro", st.serve_intensity}, {"size", st.size}, {"weight", st.weight}};
}
void from_json(const json &j, TrafficClass &st)
{
  st.serve_intensity = Intensity(j.at("micro"));
  st.size = Size(j.at("size"));
  st.weight = Weight(j.at("weight"));
}

void to_json(json &j, const Source &st);
void from_json(const json &j, Source &st);

void to_json(json &j, const Group &st);
void from_json(const json &j, Group &st);

Topology parse_topology_config(std::string_view /* filename */)
{
  return {};
}

} // namespace Config
