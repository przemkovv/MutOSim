#!/bin/sh
while true; do
    find src/ | entr -dcr ctags --options=.ctags src/
done
