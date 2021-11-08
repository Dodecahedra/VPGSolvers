# VPGSolvers
[![run-tests](https://github.com/Dodecahedra/VPGSolvers/actions/workflows/release.yaml/badge.svg)](https://github.com/Dodecahedra/VPGSolvers/actions/workflows/release.yaml)

Variability Parity games are a generalisation of parity games that can be used to verify products in a Software 
Product Line. This repository contains the implementations of all the algorithms described in the thesis 
``[New algorithms and heuristics for Variability Parity Games](https://www.dropbox.com/s/ibrzt0mpanw8tx5/VPGs.pdf?dl=0)''.
The definitions and implementations of the algorithms[^1] can be found in the `./Algorithms` directory, and include:
- Priority Promotion (`PP`)
- Recursive algorithm with tight SCC integration (`SCC`)
- Small Progress Measures (`PM`)
## Building
To build and use the solvers, first checkout the repository including the submodule with (does require a git ssh key)
``` sh
git clone --recurse-submodules https://github.com/Dodecahedra/VPGSolvers.git
```


Next we'll need `cmake`, [`BuDDy`](https://sourceforge.net/projects/buddy/) (v2.4) and a `C++14` compliant compiler
to build the binaries.

After installing BuDDy (we recommend using the default library install paths `/usr/local/include` and `/usr/local/lib`)
the binaries can be build with the command:
``` sh
cmake -S . -B build
cmake --build build
```

This produces the `VPGSolvers` binary which can be used to solve VPGs. For more info run `VPGSolver -h`.

## Tests
All algorithms are automatically integration tested using [`GTest`](https://github.com/google/googletest) on a small
sample of VPGs. The command above will also compile a test binary (`VPG_tests`) which can locally run the tests, or can be run from 
the `CLion` IDE using `CTest`.

## Miscellaneous files
The [`games.zip`](./games.zip) file contains all the individual games used when running experiments to compare the different
algorithms (results of which can be found here 
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.5637419.svg)](https://doi.org/10.5281/zenodo.5637419)).


[^1]: This repository uses [VariabilityParityGames](https://github.com/SjefvanLoo/VariabilityParityGames) by Sjef van Loo
as a base for representing and parsing Variability Parity Games.
