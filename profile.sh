#!/bin/bash

(
    printf 'uci\nsetoption name Hash value 64\nucinewgame\nisready\n\nposition startpos\ngo wtime 180000\n'
    printf 'position fen rn1qkb1r/1pp2pP1/3p4/p5Bp/4P3/2N5/PPP2PPP/R2bKB1R b KQkq - 0 9\ngo btime 60000\n'
    printf 'position fen rn2kb2/p3p1p1/2p1r3/1P2q3/3PBPb1/4Q2p/1P1P2PP/R1B1K2R w KQq - 0 17\ngo wtime 60000\n'
    printf 'position fen 8/8/7k/8/8/7K/1P6/8 b - - 0 1\ngo btime 10000\n'
    printf 'setoption name UCI_Chess960 value true\nucinewgame\n'
    printf 'position fen qrnkrbbn/pppppppp/8/8/8/8/PPPPPPPP/RQNKBBRN w AGbe - 0 1\ngo wtime 20000\n'
    printf 'quit\n'
) | "$1"
