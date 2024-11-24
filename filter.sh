#!/bin/bash

set -e

mkdir -p filtered
sort -u -k 1,4 samples/2024{09*,10*,11*}.csv | shuf > "filtered/$(date '+%Y%m%d-%H%M%S').csv"
