#!/bin/bash

set -e

#branch=$(git branch --show-current)
branch=main

c-chess-cli \
    -engine cmd="./branches/seawall-${branch}" \
    -engine cmd="./branches/seawall-main" \
    -each option.Hash=4 tc=1+0.01 \
    -openings file=./books/training.fen order=random \
    -games 1500 -concurrency 6 \
    -draw number=20 count=5 score=10 \
    -sample freq=1 decay=0 resolve=y file="samples/$(date '+%Y%m%d-%H%M%S').csv" format=csv
