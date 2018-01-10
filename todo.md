
# Todo

- Reserved capacity
- divide simulation into epochs?
- Analytical model MIMS
- graphs
- elastic traffic
- adaptive traffic
- verify global block time
- implement analytical model Fredericks-Hayward
- plotting
- statistics

# Done

- verify Pascal stream correctness
- Big decimal - using GMP, MPFR libraries
- Block time per traffic class
- fix math computing
- Service intensity per class
- add traffic class id


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
