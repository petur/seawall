#!/bin/bash

set -e

mkdir -p filtered
sort -u -k 1,4 pgn/samples/*.csv | shuf > "filtered/$(date '+%Y%m%d-%H%M%S').csv"
