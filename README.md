# seawall

This is an [UCI] chess engine. It started as an attempt to build a minimal program that was able to play chess. Over
time, it has gained features to make it stronger, but it still implements only a subset of UCI necessary to play
games, and there's almost no evaluation.

Some ideas have been borrowed more directly than others; the obstruction difference algorithm is based on the version
from [Chess Movegen], while the use of a low pass filter and capture history for move ordering were borrowed from
[Stockfish].

## Building

To build seawall, run `make` in the source directory.

By default, this will create an executable optimized for the current platform. See the `Makefile` for options that can
be used to configure the build.

The build only requires Make and a C++17 compiler. It has only been tested with GNU Make and GCC.

[uci]: https://www.chessprogramming.org/UCI
[chess movegen]: https://github.com/Gigantua/Chess_Movegen
[stockfish]: https://github.com/official-stockfish/Stockfish
