# SimForth as library

SimForth is compiled as:
* a standalone Forth interpreter in the same way than gforth, pforth
* and a shared library to be embedded in your C/C++ project if you desire
to have a Reverse Polish Notation script language.

This document will focus on the second point.

## Compilation, Installation

The shared library is automatically compiled and installed with the standalone binary.
See the [install](install.md) file.

## Examples

You can easily embed a Forth interpreter in your C/C++ project and interact with
it. This kind of script can replace Lua. This basic example
https://github.com/Lecrapouille/LinkAgainstMyLibs/blob/master/Forth/src/main.cpp
shows you how your C++ code interacts with SimForth. To link SimForth against
your application you should use the `pkg-config` tool: `pkg-config --cflags
simforth` (compile flags) and `pkg-config --libs simforth` (linker flags).

You can extend the Forth interpeter using C++ inheritance. Here an example:
https://github.com/Lecrapouille/LinkAgainstMyLibs/blob/master/ExtendedForth/src/main.cpp
