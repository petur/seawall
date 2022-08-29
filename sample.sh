#!/bin/bash

set -e

branch=$(git branch --show-current)
today=$(date '+%Y%m%d')

c-chess-cli \
    -engine cmd="./out/seawall-${today}-${branch}" \
    -engine cmd=./out/seawall-${today} \
    -each option.Hash=16 tc=1+0.01 \
    -openings file=./books/training.fen order=random \
    -games 500 -concurrency 2 \
    -sample freq=0.24 decay=0.005 resolve=y file="samples/$(date '+%Y%m%d-%H%M%S').csv" format=csv
