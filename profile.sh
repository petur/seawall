#!/bin/bash

(
    printf 'uci\nsetoption name Hash value 16\nucinewgame\nisready\n\nposition startpos\ngo wtime 3600000\n'
    sleep 8
    printf 'stop\n'
    printf 'position fen rn1qkb1r/1pp2pP1/3p4/p5Bp/4P3/2N5/PPP2PPP/R2bKB1R b KQkq - 0 9\ngo btime 3600000\n'
    sleep 2
    printf 'stop\n'
    printf 'position fen rn2kb2/p3p1p1/2p1r3/1P2q3/3PBPb1/4Q2p/1P1P2PP/R1B1K2R w KQq - 0 17\ngo wtime 3600000\n'
    sleep 2
    printf 'stop\nquit\n'
) | "$1"
