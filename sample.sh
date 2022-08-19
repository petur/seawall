#!/bin/bash

set -e

branch=$(git branch --show-current)

c-chess-cli \
    -engine cmd="./out/seawall.${branch}" name="${branch}" \
    -engine cmd=./out/seawall.main name="main" \
    -each option.Hash=16 tc=1+0.01 \
    -openings file=./books/training.fen order=random \
    -games 500 -concurrency 2 \
    -sample freq=0.3 decay=0.005 resolve=y file="samples/$(date '+%Y%m%d-%H%M%S').csv" format=csv
