#!/bin/bash

set -e

branch=$(git branch --show-current)

c-chess-cli \
    -engine cmd="./branches/seawall-${branch}" \
    -engine cmd="./branches/seawall-main" \
    -each option.Hash=4 tc=1+0.01 \
    -openings file=./books/training.fen order=random \
    -games 500 -concurrency 2 \
    -sample freq=0.24 decay=0.005 resolve=y file="samples/$(date '+%Y%m%d-%H%M%S').csv" format=csv
