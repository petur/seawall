#!/bin/bash

diff -u --color \
    <(cat <<EOF
id name seawall
id author petur
uciok
readyok
info string fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
info string fen rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1
EOF
    ) \
    <(./seawall <<EOF
uci
debug on
ucinewgame
isready
position startpos
position startpos moves e2e4
quit
EOF
    )

