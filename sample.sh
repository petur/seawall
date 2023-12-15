#!/bin/bash

set -e

branch=$(git branch --show-current)

c-chess-cli \
    -engine cmd="./branches/seawall-${branch}" \
    -engine cmd="./branches/seawall-main" \
    -each option.Hash=4 tc=1+0.01 \
    -openings file=./books/training.fen order=random \
    -games 1500 -concurrency 6 \
    -draw number=60 count=10 score=2 \
    -sample freq=0.21 decay=0.016 resolve=y file="samples/$(date '+%Y%m%d-%H%M%S').csv" format=csv
