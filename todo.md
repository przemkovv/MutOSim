
# Poprawki książka

- s. 122 BBU -> JA
- symbol w tabeli 3.7 nu -> upsilon https://www.sharelatex.com/learn/List_of_Greek_letters_and_math_symbols
- s. 142, tabela 3.8. Liczba mnoga -> liczba pojedyncza

# Publication experiments

 - Topologies:
   - 3 primary groups:
     - no mutual overflows
     - one group mutually overflowing to other
     - all three mutually overflowing to each other
   - 3 primary groups, 1 secondary group
     - no mutual overflows
     - one group mutually overflowing to other primary groups
     - all three mutually overflowing to each other primary group
 - Mutual overflow policies:
   - highest capacity available
   - lowest capacity available
   - random available
 - Compression:
   - no compression
   - adaptive without modification of serve intensity
   - adaptive with modification of serve intensity
 - Offered intensity factor for one group:
   - 1.0
   - 1.2
   - 1.5
  - nierównomierne obciążenie wiązek
  - porównanie różnych nierównomiernych obciążeń
  - dopasowanie rozmiarów zgłoszeń (nie 149, tylko coś związanego z UMTS)
  - filtrowanie jaka wiązka przyjmuje jakie typ żądań

  Other ideas:
   - compare scenarios with mutual overflows and with adaptive traffic.

# Todo

- Use cases:
  - support for multiple next groups
  - meta traffic class (for blocking time stats only)
  - traffic class priorities
  - subgroups
  - reserved capacity

- Statistics
  - verify global block time
  - distribution of used channels in group

- Calculations: +1
  - Analytical model MIMS
  - implement analytical model Fredericks-Hayward

- Technical:
  - chaining policies, e.g.
    - select_available ->  limit_groups_per_layer -> select highest_capacity_groups,
    - select_available ->  limit_groups_per_layer -> select random
  - divide simulation into epochs?
  - end simulation when there is enough number of lost requests
  - introduce ServeIntensity and OfferIntensity strong types.
  - implement inclusion system in scenario specification
  - properly serve 'a' offered intenstity with various intensities multipliers
    in plot.py

# Done

- Use cases:
  - filter groups by traffic class
  - compression (existing / only new). thresholds per tc and per group
  - elastic traffic
  - adaptive traffic

- Experiments:
  - differentiate tc more
  - capacities differentation

- Technical
  - graphviz to display topology

- Statistics
  - forwarding stats

- statistics: export to json
- plotting
- max two overflows
- record load path (by what groups it was served)
- different intensities per group (intensity_multiplier)
- scenarios with mutual overflows (3 primary - one secondary)
- verify Pascal stream correctness
- Big decimal - using GMP, MPFR libraries
- Block time per traffic class
- fix math computing
- Service intensity per class
- add traffic class id


# Notes

## Computing offered traffic intensity

Should be all topology be taken into account?


# Dictionary

- lambda - intensywność zgłoszeń - (ang. request intensity)
- micro  - intensywność obsługi - (ang. serve intensity)
- A - natężenie ruchu - (ang. traffic intensity)


