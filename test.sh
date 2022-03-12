#!/bin/bash

diff -u --color \
    <(cat <<EOF
id name seawall
id author petur
uciok
readyok
info string fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
info string fen rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1
info string fen r1bqkbnr/pppppppp/2n5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 1
info string fen rnbqk2r/pppp1ppp/3b1n2/4p3/4P3/3B1N2/PPPP1PPP/RNBQ1RK1 b kq - 5 1
EOF
    ) \
    <(./seawall <<EOF
uci
debug on
ucinewgame
isready
position startpos
position startpos moves e2e4
position startpos moves e2e4 b8c6
position startpos moves e2e4 e7e5 g1f3 g8f6 f1d3 f8d6 e1g1
quit
EOF
    )

