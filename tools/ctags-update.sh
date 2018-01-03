while true; do find src/ | entr -dc make -C build/ ctags; done
