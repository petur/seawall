#!/bin/bash

set -e

mkdir -p filtered
sort -u -k 1,4 pgn/samples/20261*.csv pgn/samples/20260[2-9]*.csv | shuf > "filtered/$(date '+%Y%m%d-%H%M%S').csv"
