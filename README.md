# VPGSolvers
[![run-tests](https://github.com/Dodecahedra/VPGSolvers/actions/workflows/release.yaml/badge.svg)](https://github.com/Dodecahedra/VPGSolvers/actions/workflows/release.yaml)

Variability [Parity games](link) are a generalisation of parity games that can be used to verify products in a Software Product Line. This repository contains the implementation of all the algorithms described in the thesis ``''.

The definitions and implementations of the algorithms can be found in the `./Algorithms` directory, and includes:
- Priority Promotion (`PP`)
- Recursive algorithm with tight SCC integration (`SCC`)
- Small Progress Measures (`PM`)

## Building
To build and use the solvers, first checkout the repository including the submodule with

``` sh
git clone --recurse-submodules https://github.com/Dodecahedra/VPGSolvers.git
```

Next we'll need `cmake` and `BuDDy` to build the binaries with

``` sh
cmake -S . -B build
cmake --build build
```

This produces the `VPGSolver` binary which can be used to solve VPGs. For more info run `VPGSolver -h`.


This Repository uses [VariabilityParityGames](https://github.com/SjefvanLoo/VariabilityParityGames) as a base for representing and parsing Variability Parity Games.
