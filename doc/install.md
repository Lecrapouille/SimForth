# Download, compile and install SimForth

SimForth can be compiled as:
* a standalone Forth interpreter in the same way than gforth, pforth
* and a shared library to be embedded in your C/C++ project if you desire
to have a Reverse Polish Notation script language.

This document explain all these points.

## Prerequisite

* Makefile, g++ >= 4.9 or clang++.
* System libraries: `sudo apt-get install libreadline-dev pkg-config bc`
* MyMakefile repo https://github.com/Lecrapouille/MyMakefile (downloaded as git sub-module).
* Some of my repos from https://github.com/Lecrapouille : TerminalColor, MyLogger, Exception.
  They are downloaded with the command `make download-external-libs`.
* In debug mode: the third part project https://github.com/bombela/backward-cpp is downloaded
  automatically when you compile SimForth.
* Optionally, if you want to run unit tests, you have to download, compile and install
  Google tests (gtest and gmock) with the same compiler that will compile SimForth.
* Optionally, gcov for code coverage.

## Download the code source

```sh
git clone git@github.com:Lecrapouille/SimForth.git --depth=1 --recurse-submodules
cd SimForth
make download-external-libs
```

The last command allows to download extra GitHub libraries that SimForth depends on.
For developpers do not add `--depth=1` to download the whole git history.

### Compilation commands

To compile the SimForth project (standalone + static and shared libraries):

```sh
cd SimForth
make -j8
```

Where 8 of `-j8` is the number of CPU cores you have (8 in my case). 
For verbose you have to add `V=1 make`. For changing of compiler use CXX `V=1 make CXX=clang++-7 -j8`.
Once compiled, the standalone project can be run by:

```
./build/SimForth
```

### Cleaning

`make clean` or `make veryclean`

### Install SimForth

You can install the project (binary + libraries + forth files + doc) in your operating system with:

```
sudo make install
```

This will install:
* `/usr/bin` the binary SimForth.
* `/usr/share/SimForth/<version>/` documentations and core SimForth files.
* `/usr/lib` and `/usr/lib/pkgconfig` library files and pkg-config file.
* `/usr/include/SimForth-<version>/` header files for libraries.

Note that several versions of SimForth can live together thanks to folder `<version>` or version value in the binary name.

### Unit tests / Non regression check

Optional if you do not want to help developping you can compile unit tests, run tests and display code coverage:

```sh
cd SimForth
make check
```

If coverage report disturb you. You can do that:

```sh
cd SimForth/tests
make
./build/SimForth-UnitTests
```
