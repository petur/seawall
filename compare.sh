#!/bin/bash

set -e

hash='4'
tc='3+0.03'
base='main'
for arg in "$@"; do
    case "$arg" in
    hash=*)
        hash="${arg#hash=}"
        ;;
    tc=*)
        tc="${arg#tc=}"
        ;;
    base=*)
        base="${arg#base=}"
        ;;
    esac
done


branch=$(git branch --show-current)

fastchess \
    -engine cmd="./branches/seawall-${branch}" name="seawall-${branch} #1" \
    -engine cmd="./branches/seawall-${base}" name="seawall-${base} #2" \
    -each option.Hash="$hash" tc="$tc" \
    -openings file=./books/testing.fen format=epd order=random \
    -rounds 1000000 -repeat \
    -concurrency 6 \
    -sprt elo0=0 elo1=4 alpha=0.05 beta=0.05 \
    -log file=compare.log \
    -pgnout file="pgn/$(date '+%Y%m%d-%H%M%S').pgn"
