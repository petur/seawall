# seawall

This is an [UCI] chess engine. It started as an attempt to build a minimal program that was able to play chess. Over
time, it has gained features to make it stronger, but it still implements only a subset of UCI necessary to play
games. The evaluation function is based on piece square tables and bitboards; a loop over the pieces is used only to
calculate mobility. The evaluation has been tuned using positions from self-play games. The UCI implementation is
single threaded, using polling to handle input during search.

Some ideas have been borrowed more directly than others; the obstruction difference algorithm is based on the version
from [Chess Movegen], while the use of a low pass filter and capture history for move ordering were borrowed from
[Stockfish].

## Building

To build seawall, run `make` in the source directory. The C++ compiler can be specified using the `CXX` variable, for
example `make CXX=clang++` will build using CLang.

By default, this will create an executable optimized for the current platform. See the `Makefile` for options that can
be used to configure the build.

The build only requires Make and a C++17 compiler. It has only been tested with GNU Make, GCC, and CLang.

[uci]: https://www.chessprogramming.org/UCI
[chess movegen]: https://github.com/Gigantua/Chess_Movegen
[stockfish]: https://github.com/official-stockfish/Stockfish
