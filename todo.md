
# Poprawki książka

- s. 122 BBU -> JA
- symbol w tabeli 3.7 nu -> upsilon https://www.sharelatex.com/learn/List_of_Greek_letters_and_math_symbols

# Todo

- Use cases:
  - compression (existing / only new). thresholds per tc and per group
  - filter groups by traffic class
  - support for multiple next groups
  - meta traffic class (for blocking time stats only)
  - traffic class priorities
  - subgroups
  - reserved capacity
  - elastic traffic
  - adaptive traffic

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

# Done

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


# Experiments results

## 3+1 groups, (60+50+40)->80, t=1,4,9, all overflows to each other

 - The alternative block probability in the alternate group is almost the same
   in each overflow policy
 - The block probability in primary groups is lower when using highest free
   capacity overflow policy, than lowest free capacity for t=1 (about 10-20%
   better). For bigger loads, there is no difference.
 - Random behaves similar to the lowest free capacity.


## 3+1 groups, (60+50+40)->80, t=1,4,9, single overflow to each other

 - When mutual overflows are enabled, all policies behaves the same. Only the
   mutually overflwoing group has lower P_block than other.
 - Without mutual overflows are groups behaves the same.

## 3+1 groups, (60+50+40)->80, t=1,4,9, all overflows to each other. G2 IM1.5

 - The alternative block probability in the alternate group is almost the same
   in each overflow policy
 - The block probability in primary groups is lower when using highest free
   capacity overflow policy, than lowest free capacity for t=1 (about 10-20%
   better). For bigger loads, there is no difference.
 - Random behaves similar to the lowest free capacity.


## 3+1 groups, (60+50+40)->80, t=1,4,9, single overflow to each other G2 IM1.5

 - When mutual overflows are enabled, all policies behaves the same. Only the
   mutually overflwoing group has lower P_block than other.
 - Without mutual overflows are groups behaves the same.
