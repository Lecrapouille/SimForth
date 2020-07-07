# SimForth

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://github.com/Lecrapouille/SimForth/blob/master/LICENSE)
[![codecov](https://codecov.io/gh/Lecrapouille/SimForth/branch/master/graph/badge.svg)](https://codecov.io/gh/Lecrapouille/SimForth)
[![coveralls](https://coveralls.io/repos/github/Lecrapouille/SimForth/badge.svg?branch=master)](https://coveralls.io/github/Lecrapouille/SimForth?branch=master)

|Branch     | **`Linux/Mac OS`** | **`Windows`** |
|-----------|------------------|-------------|
|master     |[![Build Status](https://travis-ci.org/Lecrapouille/SimForth.svg?branch=master)](https://travis-ci.org/Lecrapouille/SimForth)|[![Build status](https://ci.appveyor.com/api/projects/status/github/lecrapouille/SimForth?svg=true)](https://ci.appveyor.com/project/Lecrapouille/SimForth)|
|development|[![Build Status](https://travis-ci.org/Lecrapouille/SimForth.svg?branch=dev-refacto)](https://travis-ci.org/Lecrapouille/SimForth)||

SimForth is a personal [Forth interpreter](https://www.forth.com/starting-forth/)
originally used in my work-in-progress project named
[SimTaDyn](https://github.com/Lecrapouille/SimTaDyn) which is a Geographic
Information System (GIS) in which maps can be manipulated as a spreadsheet and
where Forth replaces the Excel's Visual Basic, the scripting language for formulas
stored in spreadsheet cells.

I am currently moving apart some modules of SimTaDyn and the Forth interpreter
is one of these parts. Now, SimForth has its own git repo and I hope have it's own
life. SimForth is compiled as:
- a basic [standalone Forth
  interpreter](https://github.com/Lecrapouille/SimForth/blob/master/doc/standalone.md)
  (in the same way than gforth, pforth, ...) offering you the choice, through the command
  line, between an interactive prompt or interpreting scripts (from files or string).
- and a shared library that allows you to embed a Forth interpreter in your personal C++
  projects like demonstrated in this complete [example](https://github.com/Lecrapouille/LinkAgainstMyLibs/blob/master/Forth/src/main.cpp).

SimForth is not a Forth conceived to be embedded in micro-controlers. It has
been developed in C++ and will offering GIS and spreadsheet extensions (in gestation).
As a Forth beginner, SimForth has its own personal touch concerning the
implemention of the interpreter, dictionary, input buffer, and some features have been discarded (source
editor, user variable, vocabulary). The core of SimForth is for the moment not complete and
some words are currently in development. As consequence, SimForth is not 100% compliant to
[ANSI-Forth 2012](https://forth-standard.org/standard/words).

Note that in SimTaDyn, a Forth editor made in GTKmm (GTK+ wrapper for C++) is present but is not yet
present in this repo.

## Installation

See [here](doc/install.md) for downloading code source, compiling them and installing binaries.

## Features

I am a beginner in Forth and SimForth is, at this stage, a basic Forth
interpreter, does not have a complete library of words and does not follow
completely the Forth 2012 standard. See [deviations](doc/deviation.md).

Implemented:
- Standalone Forth. See [here](src/standalone/).
- Basic GTK+ spreadsheet standalone using SimForth instead of Visual Basic. See [here](src/spreadsheet/).
- Shared library for your C++ project desiring to embed a Forth interpreter.
- Interactive mode, interpret files and C++ strings.
- Auto-completion of words in the interactive mode.
- History of commands in the interactive mode.
- Minimal Forth system (IF ELSE THEN, LOOP, CREATE, <BUILDS, DOES>, INCLUDE, .", ABORT, VALUE, ARRAY ...). See [here](core/System/Core.fth).
- [Glossary](doc/glossary.md).
- Basic self-tests made in Forth. See [here](core/SelfTests/tests-core.fth).
- Can execute basic Legacy OpenGL code. See [here](core/OpenGL/OpenGL.fth).
- Data-Stack mixing integer and float values (no separated floating point stack).
- No Return-Stack manipulation but Auxiliary-Stack manipulation).
- Can import C code and interface C functions from external libraries.
- Display colorful and human-friendly error messages (more than gforth).
- Display colorful dictionary. See [here](img/Dictionary.png).
- Basic colorful debugger. See [here](img/Debugger.png).
- Max number of characters for a word is 32. Unicode words can be accepted while this feature is not really tested.
- Save and load dump dictionary.
- Unit tests and continuous integration scripts.

To be done:
- Missing words (in gestation): locals, throw, catch, multi-tasking ...
- Forth structures not compatible with C structure.
- Little/Big endian compatibility when dictionary is saved. For the moment only
AMD64-style architecture is managed.

Known issues:
- Disastrous dictionary byte manipulation.
- Save/load dictionnary content having C functions loaded from sahred libraries.

Work in progress:
- See [here](https://github.com/Lecrapouille/SimForth/issues/1)

## Why SimForth?

* Why SimForth? When I started to develop SimTaDyn I wanted a scripting language
  with GIS primitives for managing my maps.

* Why not using Lua or Python or Julia? Because Forth has no syntax (thanks to
  the Reverse Polish notation) and therefore, a Forth script is enough
  minimalist to be embedded in cells of spreadsheets in the same way of Excels'
  Basic. Minimalist does not mean weak functionalities! Forth is coming from
  embedded systems in 70's and has been used in astronomy, it has a very tiny
  footprint memory: up to 64 KiB for classic Forth. This size includes the interpreter
  and the virtual machine holding the byte code. This reduced size is possible mainly because the
  interpreter does not have to manage complex language syntax. In addition, this language
  can self-evolving its syntax, living in a multi-tasking virtual machine that
  can easily be dumped in a file and shared. Lua or Python or Julia are a size
  more complex while offering a more friendly syntax and powerful libraries.

* Why developing another Forth interpreter? Why not simply using gforth?  I
  wanted to learn Forth after my father gave me the book "FORTH" by
  W. P. Salman, O. Tisserand and B. Toulout, Editions Eyrolles, 1983.  SimTaDyn
  started as a one-year student project and making a Forth interpreter was a
  good idea to impress my teachers (while, in fact, they were not all impressed
  by the beauty of Forth ... and by SimTaDyn). The other reason was the code
  source of gforth is too obscure for me, pforth can easily be crashed and I did
  not know at this time the uncrashable 4th. In addition, I wanted to develop my own Forth
  managing my desired features and, as a noob, to implement core features in C
  and interface them with Forth words. They usually do that in Python so why not
  in Forth!

* Why using C++ for developing Forth? Should not be better if written in
  assembly? Yes, assembly would have been a better choice but I wanted to have a
  portable Forth and use the inheritance of C++ to implement easily different
  Forth interpreters (from classic Forth to a specialized Forth for GIS).

## Why using SimForth?

SimForth is the scripting language for SimTaDyn and should be used from SimTaDyn.
SimForth, used as standalone, is less powerful than any serious Forth interpreter
and it is two times slower than pforth. See [benchmark](tests/bench).
SimForth, used as shared library, offers a simpler API than gforth, in my opinion.

## Documentation and Inspiration

SimForth has some documentation inside the folder [docs](doc):
* [doc/install.md](doc/install.md) shows how to compile, install and use SimForth.
* [docs/standalone.md](doc/standalone.md) show how to use the standalone binary.
* [docs/libsimforth.md](doc/libsimforth.md) show how to use the libsimforth library.
* [docs/learning.md](doc/learning.md) links for learning Forth.
* [docs/simforth.md](doc/simforth.md) learning SimForth.
* [docs/deviation.md](doc/deviation.md) shows deviation from ANSI-Forth.
