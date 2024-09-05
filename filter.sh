#!/bin/bash

set -e

mkdir -p filtered
sort -u -k 1,4 samples/2024*.csv | shuf > "filtered/$(date '+%Y%m%d-%H%M%S').csv"
