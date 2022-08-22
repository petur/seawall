#!/bin/bash

set -e

tc='3+0.03'
for arg in "$@"; do
    case "$arg" in
        tc=*)
            tc="${arg#tc=}"
            ;;
    esac
done


branch=$(git branch --show-current)
today=$(date '+%Y%m%d')

c-chess-cli \
    -engine cmd="./out/seawall-${today}-${branch}" \
    -engine cmd=./out/seawall-${today} \
    -each option.Hash=256 tc="$tc" \
    -openings file=./books/testing.fen order=random \
    -games 20000 -repeat \
    -concurrency 2 -log -sprt
