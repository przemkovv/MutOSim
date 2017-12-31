

on_produce(t):
 - produce new load and send (t)
 - if produced for linked source, extend linked source on LoadSend

on_send(t):
 - if load is from closing source do nothing ???
 - else schedule produce new load (t+dt)

on_accept(t):
 - schedule linked produce new load (t+dt)

on_drop(t):
 - if load created new source

on_served(t):
 - if linked produce new load exists: remove




