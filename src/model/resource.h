
#pragma once

#include "types/types.h"

#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/transform.hpp>
#include <vector>

namespace Model {
template <typename C = Capacity>
struct ResourceComponent
{
  Count number; // v_s
  C     v;      // f_s
  C     V() const { return number * v; }

  template <typename C2>
  operator ResourceComponent<C2>() const
  {
    return ResourceComponent<C2>{number, C2(v)};
  }
};

template <typename C>
ResourceComponent<CapacityF>
operator/(const ResourceComponent<C> &component, Peakedness peakedness)
{
  return {component.number, CapacityF{component.v / peakedness}};
}

template <class C>
ResourceComponent(Count, C)->ResourceComponent<C>;

template <typename C = Capacity>
struct Resource
{
  std::vector<ResourceComponent<C>> components{};

  Resource() = default;
  Resource(C capacity) : Resource({{Count{1}, capacity}}) {}
  Resource(Count count, C capacity) : Resource({{count, capacity}}) {}
  Resource(std::vector<ResourceComponent<C>> resourceComponents)
    : components(std::move(resourceComponents))
  {
  }
  Resource(std::initializer_list<ResourceComponent<C>> initializer_list)
    : components(initializer_list)
  {
  }
  Resource(const std::vector<C> &capacities)
    : components(capacities | ranges::view::transform([](C capacity) {
                   return ResourceComponent{Count{1}, capacity};
                 }))
  {
  }

  Resource(std::initializer_list<C> initializer_list)
    : components(initializer_list | ranges::view::transform([](C capacity) {
                   return ResourceComponent{Count{1}, capacity};
                 }))
  {
  }

  C V() const
  {
    return ranges::accumulate(
        components, C{0}, [](const C &V, const ResourceComponent<C> &rc) {
          return V + rc.V();
        });
  }
  template <typename C2>
  operator Resource<C2>() const
  {
    return Resource<C2>(
        components | ranges::view::transform([&](const auto &component) {
          return ResourceComponent<C2>(component);
        })
        | ranges::to_vector);
  }

  void add_component(C capacity, Count number = Count{1})
  {
    if (auto it = ranges::find_if(
            components,
            [&](const auto &component) { return component.v == capacity; });
        it != end(components))
    {
      it->number += number;
    }
    else
    {
      components.emplace_back(ResourceComponent<C>{number, capacity});
    }
  }
};

template <typename C>
Resource<CapacityF>
operator/(const Resource<C> &resource, Peakedness peakedness)
{
  return {resource.components | ranges::view::transform([&](auto component) {
            return component / peakedness;
          })};
}

} // namespace Model
