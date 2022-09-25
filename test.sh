#!/bin/bash

set -e

diff -u --color \
    <(cat <<EOF
info string fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
info string fen rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1
info string fen r1bqkbnr/pppppppp/2n5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 1
info string fen rnbqk2r/pppp1ppp/3b1n2/4p3/4P3/3B1N2/PPPP1PPP/RNBQ1RK1 b kq - 5 1
info string fen rnbqk2r/pppp1ppp/3b1n2/4p3/4P3/3B1N2/PPPP1PPP/RNBQ1K1R b kq - 5 1
info string fen rnbqk2r/pppp1ppp/3b1n2/4p3/4P3/3B1N2/PPPP1PPP/RNBQKR2 b Qkq - 5 1
info string fen R3k2r/8/8/8/8/8/8/4K2R b Kk - 0 1
info string fen rnbqkbnr/pp2pppp/3P4/2p5/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1
info string fen Q6k/8/8/8/8/8/8/7K b - - 0 1
EOF
    ) \
    <(./seawall <<EOF |
uci
debug on
ucinewgame
isready
position startpos
position startpos moves e2e4
position startpos moves e2e4 b8c6
position startpos moves e2e4 e7e5 g1f3 g8f6 f1d3 f8d6 e1g1
position startpos moves e2e4 e7e5 g1f3 g8f6 f1d3 f8d6 e1f1
position startpos moves e2e4 e7e5 g1f3 g8f6 f1d3 f8d6 h1f1
position fen r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1 moves a1a8
position startpos moves e2e4 c7c5 e4e5 d7d5 e5d6
position fen 7k/P7/8/8/8/8/8/7K w - - 0 1 moves a7a8q
quit
EOF
    grep '^info')

(
    echo '
uci
ucinewgame
isready'
echo 'position fen rn1qkb1r/1pp2pP1/3p4/p5Bp/4P3/2N5/PPP2PPP/R2bKB1R b KQkq - 0 9'
echo 'go wtime 2404 winc 30 btime 2503 binc 30'
echo 'position fen rn2kb2/p3p1p1/2p1r3/1P2q3/3PBPb1/4Q2p/1P1P2PP/R1B1K2R w KQq - 0 17'
echo 'go wtime 2095 winc 30 btime 1849 binc 30'
echo 'quit'
) | ./seawall
