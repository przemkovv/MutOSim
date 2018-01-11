
# Todo

- subgroups
- Reserved capacity
- divide simulation into epochs?
- Analytical model MIMS
- elastic traffic
- adaptive traffic
- verify global block time
- implement analytical model Fredericks-Hayward
- plotting
- statistics: export to CSV

# Done

- verify Pascal stream correctness
- Big decimal - using GMP, MPFR libraries
- Block time per traffic class
- fix math computing
- Service intensity per class
- add traffic class id

# Ideas

- record load path (by what groups it was served)
- support for multiple next groups
- filter groups by traffic class
- graphviz to display topology


# Pytania

- jak działa strata przy przelewie. Kto powinien notować stratę pierwotny czy wtórny zasób?
 - jak traktować intensywność lambda/gamma, głównie chodzi o wzory
 - Topologia single_overflow_multiple_poissonX.json. Który poprawny czas
   blokady

$ Statystyki:

- Group { TC { serve,loss, serve_u,loss_u, P_loss, P_loss_u, block_time,
  duration, P_block} }

# Słownik

- lambda - intensywność zgłoszeń - (ang. request intensity)
- micro  - intensywność obsługi - (ang. serve intensity)
- A - natężenie ruchu - (ang. traffic intensity)
