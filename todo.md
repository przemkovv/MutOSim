
# Poprawki książka

- wzór 3.23 - na pewno powinno być dwa razy d_{c,s}?
- wzór 3.8 - brak granicy sumy
- sekcja 3.5.2.6, s. 107 - brak referencji - znaki zapytania ?? - odwołania do
  rysunku 3.28
- rysunki 3.27 i 3.28  odnoszą się do metody Overflow-Conv, w tekściej jest
  Overflow-MIM

- s. 122 BBU -> JA
- symbol w tabeli 3.7 nu -> upsilon https://www.sharelatex.com/learn/List_of_Greek_letters_and_math_symbols
- s. 142, tabela 3.8. Liczba mnoga -> liczba pojedyncza
- s. 97, stopka: przyjęto -> przyjęte

# Issues

 - Czy różnego typu warstwy (FAG,LAG, itp.) są ze sobą kompatybilne w metodzie
   F-H?
 - Łączenie modeli niepełnodostępnych wraz z ruchem Er-En-Pa
 - Efektywność modelu EIR
 - Metoda Sanders w teletraffic handbook
 - Artykuły


# Todo

- Compare influence of different capacities/ number of streams/ number of
  primary groups on errors.

Fix:
 - consider peakedness of the traffic class not aggregated  in FixedCapacity
   method



 Analytical models:
 - Wiązka z ograniczoną dostępnością 75
  - metoda Sandersa porównanie z Haywarded -- znaleźć
  - model analityczny - podgrupy, LAG, s.212, prof 3.4.2.1, równe pojemność


- Verify Q
  - What happens when peakness is negative?
  - What happens when peaknes is less than 1.0?
  - Apply criterion on block probability fit 3.3.2.3.2
  - Hierarchical networks and Frederick-Hayward and peakness and original
    source streams

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
  - implement analytical model of Sanders

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

- Analytical methods
  - (1) V/Z vs. (2) Z*t: Metoda (1) jest dokładniejsza od (2) w przypadku
    małych pojemności (<1000). I dokładność jest praktycznie stała. Metoda (2)
    dla niskich objętości ma błąd większy aniżeli (1), jednakże dla dużych się
    poprawia. Dla (2) zaokrąglanie (podłoga) jest opóźniona jak najbardziej.

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
   - compare scenarios with mutual overflows and with threshold mechanisms.

  Scenariusze dlaczego kompresja:
   - kompresja jednej klasy ruchu, ale wszędzie
   - kompresja w jednej grupie  - zostaniesz obsłużony przez kogo innego, ale z
     kompresją


# Notes

## Computing offered traffic intensity

Should be all topology be taken into account?


# Dictionary

- lambda - intensywność zgłoszeń - (ang. request intensity)
- micro  - intensywność obsługi - (ang. serve intensity)
- A - natężenie ruchu - (ang. traffic intensity)


