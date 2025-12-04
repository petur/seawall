#!/usr/bin/env -S hatch run
# /// script
# requires-python = ">=3.11"
# dependencies = [
#   "chess>=1.11.2",
# ]
# ///

import os
import os.path
import sys
import chess
import chess.pgn

def pgn2samples(pgn, samples):
    while True:
        game = chess.pgn.read_game(pgn)
        if not game:
            break

        result = game.headers["Result"]
        result = chess.WHITE if result == "1-0" else chess.BLACK if result == "0-1" else None

        while True:
            next = game.next()
            if not next:
                break

            board = game.board()
            if not board.is_capture(next.move):
                try:
                    score = int(float(next.comment.split("/")[0]) * 100)
                    r = "2" if result == board.turn else "1" if result is None else "0"
                    print(f"{board.fen()},{score},{r}", file=samples)
                except:
                    pass
            game = next


if __name__ == "__main__":
    for f in os.listdir("pgn"):
        if not f.endswith(".pgn"):
            continue
        filename = os.path.join("pgn", f)
        if not os.path.isfile(filename):
            continue

        outfilename = os.path.join("pgn", "samples", f.removesuffix(".pgn") + ".csv")
        if os.path.isfile(outfilename):
            continue
        print(f"{filename} -> {outfilename}")
        with open(filename) as pgn:
            with open(outfilename, "a") as samples:
                pgn2samples(pgn, samples)
