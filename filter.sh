#!/bin/bash

set -e

mkdir -p filtered
sort -u -k 1,4 samples/{202412*,202501*,202502*}.csv | shuf > "filtered/$(date '+%Y%m%d-%H%M%S').csv"
