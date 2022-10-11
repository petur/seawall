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

c-chess-cli \
    -engine cmd="./branches/seawall-${branch}" \
    -engine cmd="./branches/seawall-${base}" \
    -each option.Hash="$hash" tc="$tc" \
    -openings file=./books/testing.fen order=random \
    -games 20000 -repeat \
    -concurrency 2 -log -sprt
