
#pragma once

#include <algorithm>
#include <random>

template <typename Container, typename T>
auto contains(const Container &container, const T &value)
{
  return find(begin(container), end(container), value) != end(container);
}

template <typename Container, typename RandomEngine>
auto get_random_element(const Container &container, RandomEngine &random_engine)
{
  std::remove_cv_t<std::remove_reference_t<decltype(*begin(container))>> elem;
  std::sample(begin(container), end(container), &elem, 1, random_engine);
  return elem;
};

template <typename GroupsContainer, typename Layer>
auto count_same_layer_groups(const GroupsContainer &path, Layer layer)
{
  return count_if(begin(path), end(path),
                  [layer](const auto &group_ptr) { return group_ptr->layer_ == layer; });
};
