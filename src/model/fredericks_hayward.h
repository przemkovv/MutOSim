#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

namespace Model
{
class FredericksHayward
{
  float variance_; //  sigma
  float mean_;     // R
  float peakness_; // Z = sigma^2/R
  float capacity_; // V

  int subsystem_number_; // ro
  float mean_fi_;        // R_fi / ro

  float traffic_loss_fi_;       // L/ro --
  float traffic_loss_coeff_fi_; // C_fi = L_fi/R_fi = L / R = C

  // equivalent system
  float variance_eqv_;

  float
  loss_probability(float primary_resources_number /* r */, float peakness, float capacity)
  {
    struct PrimaryResource {
      float intensity;   // A
      float capacity;    // V
      float variance_sq; // ro^2
      float peakness;    // Z
    };

    std::vector<PrimaryResource> primary_resources(primary_resources_number);

    // 1. Określenie wartości średniej i wariancji każdego z r strumieni ruchu
    // spływającego na zasób alternatywny (2.15), (2.16)
    for (auto &pr : primary_resources) {
      float mean = pr.intensity * erlang_block_probability(pr.intensity, pr.capacity);
      pr.variance_sq =
          mean * (pr.intensity / (pr.capacity + 1 - pr.intensity + mean) + 1 - mean);
      pr.peakness = pr.variance_sq / mean;
    }
    // 2. Określenie parametrów całkowitego strumienia spływającego na rozważany zasób
    // alternatywny przy założeniu statystycznej niezależności spływających strumienie, tj
    // . warości średniej R i wariancji Ro^2 - wzór (2.46) oraz współczynnika degeneracji
    // Z - wzór (2.20)

    float mean =
        std::accumulate(begin(primary_resources), end(primary_resources), 0f,
                        [](float mean, const auto &pr) { return mean + pr.mean); });
    float variance_sq = std::accumulate(
        begin(primary_resources), end(primary_resources), 0f,
        [](float variance_sq, const auto &pr) { return variance_sq + pr.variance_sq); });

    float = // Z_v
        1 - intensity * (erlang_block_probability(intensity, capacity - 1) -
                         erlang_block_probability(intensity, capacity));
    // 3. Określenie wymaganej pojemności zasobu alternatywnego przy zadanej jakości
    // obsługi równej E - wzór (2.73)
  }
};

} // namespace Model
