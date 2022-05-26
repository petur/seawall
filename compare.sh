#!/bin/bash

set -e

branch=$(git branch --show-current)

c-chess-cli \
    -engine cmd="./out/seawall.${branch}" name="${branch}" \
    -engine cmd=./out/seawall.main name="main" \
    -each option.Hash=256 tc=0.4+0.03 \
    -openings file=./books/testing.fen \
    -games 800 -repeat \
    -concurrency 2 -log -sprt
