
#pragma once


#include <algorithm>
#include <nlohmann/json.hpp>
#include <range/v3/algorithm/count_if.hpp>
#include <range/v3/algorithm/find.hpp>
#include <string>
#include <vector>

namespace rng = ranges;

template <typename Container, typename T>
auto
contains(const Container &container, const T &value) -> bool
{
  return rng::find(container, value) != std::end(container);
}

template <typename BeginIt, typename EndIt, typename RandomEngine>
auto
get_random_element(BeginIt &&begin, EndIt &&end, RandomEngine &random_engine)
{
  std::remove_cv_t<std::remove_reference_t<decltype(*begin)>> elem;
  std::sample(begin, end, &elem, 1, random_engine);
  return elem;
}

template <typename Container, typename RandomEngine>
auto
get_random_element(const Container &container, RandomEngine &random_engine)
{
  return get_random_element(begin(container), end(container), random_engine);
}

template <typename GroupsContainer, typename Layer>
auto
count_same_layer_groups(const GroupsContainer &path, Layer layer)
{
  return rng::count_if(path, [layer](const auto &group_ptr) { return group_ptr->layer_ == layer; });
}

nlohmann::json concatenate(nlohmann::json target, const nlohmann::json &patch);

std::string join(const std::vector<std::string> &strings, const std::string &separator);
