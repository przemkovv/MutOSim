#!/bin/sh
while true; do
    find src/ | entr -dcrs 'date; ctags --options=.ctags src/'
done
