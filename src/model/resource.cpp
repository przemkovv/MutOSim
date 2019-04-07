
#include "resource.h"

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/transform.hpp>

namespace rng = ranges;

namespace Model {
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Capacity
// Resource::V() const
// {
  // return rng::accumulate(
      // components, Capacity{0}, [](const Capacity &V, const ResourceComponent &rc) {
        // return V + rc.V();
      // });
// }

// Resource::Resource(const std::vector<Capacity> &capacities)
  // : components(capacities | rng::view::transform([](Capacity capacity) {
                 // return ResourceComponent{Count{1}, capacity};
               // }))
// {
// }

// Resource::Resource(std::initializer_list<Capacity> initializer_list)
  // : components(initializer_list | rng::view::transform([](Capacity capacity) {
                 // return ResourceComponent{Count{1}, capacity};
               // }))
// {
// }
} // namespace Model
