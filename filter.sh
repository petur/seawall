#!/bin/bash

set -e

mkdir -p filtered
sort -u -k 1,4 samples/{202502*,202503*,202508*}.csv pgn/samples/*.csv | shuf > "filtered/$(date '+%Y%m%d-%H%M%S').csv"
