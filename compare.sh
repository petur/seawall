#!/bin/bash

branch=$(git branch --show-current)

c-chess-cli \
    -engine cmd="./out/seawall.${branch}" name="${branch}" \
    -engine cmd=./out/seawall.main name="main" \
    -each option.Hash=256 tc=0.2+0.01 \
    -openings file=./books/testing.fen \
    -games 10 -repeat \
    -concurrency 2 -log -sprt
