# SimForth

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://github.com/Lecrapouille/SimForth/blob/master/LICENSE)
[![codecov](https://codecov.io/gh/Lecrapouille/SimForth/branch/master/graph/badge.svg)](https://codecov.io/gh/Lecrapouille/SimForth)
[![coveralls](https://coveralls.io/repos/github/Lecrapouille/SimForth/badge.svg?branch=master)](https://coveralls.io/github/Lecrapouille/SimForth?branch=master)

|Branch     | **`Linux/Mac OS`** | **`Windows`** |
|-----------|------------------|-------------|
|master     |[![Build Status](https://travis-ci.org/Lecrapouille/SimForth.svg?branch=master)](https://travis-ci.org/Lecrapouille/SimForth)|[![Build status](https://ci.appveyor.com/api/projects/status/github/lecrapouille/SimForth?svg=true)](https://ci.appveyor.com/project/Lecrapouille/SimForth)|
|development|[![Build Status](https://travis-ci.org/Lecrapouille/SimForth.svg?branch=dev-refacto)](https://travis-ci.org/Lecrapouille/SimForth)||

[SimForth](https://github.com/Lecrapouille/SimForth) is a non serious and
personal [Forth interpreter](https://www.forth.com/starting-forth/) in the
category of [token forth](http://www.bradrodriguez.com/papers/moving1.htm]). It
has been developed in C++11,

SimForth is compiled and installed as:
* A basic standalone Forth interpreter (in the same way than gforth,
  pforth, ...), running on the console and offering you the choice between an
  interactive prompt or interpreting scripts (from files or string). For more
  information, see [the
  document](https://github.com/Lecrapouille/SimForth/blob/master/doc/standalone.md)
* A shared library that allows you to embed a Forth interpreter in your personal
  C++ projects like shown in this
  [example](https://github.com/Lecrapouille/LinkAgainstMyLibs/blob/master/Forth/src/main.cpp). You
  can extend the original Forth C++ class to create a personal Forth like shown
  in this [example](https://github.com/Lecrapouille/LinkAgainstMyLibs/blob/master/ExtendedForth/src/main.cpp).
* A Forth IDE made in GTK+ (work in progress).
* A proof of concept standalone tiny spreadsheet tool using Forth instead of
  Visual Basic and made in GTK+.

![doc/img/Debugger.png](doc/img/Debugger.png)

## Documentation

SimForth comes with some documentation inside the folder [doc](doc):
* [doc/install.md](doc/install.md) shows how to compile, install and use SimForth.
* [doc/standalone.md](doc/standalone.md) show how to use the standalone binary.
* [doc/libsimforth.md](doc/libsimforth.md) show how to use the libsimforth library.
* [doc/learning.md](doc/learning.md) links for learning Forth.
* [doc/simforth.md](doc/simforth.md) learning SimForth.
* [doc/deviation.md](doc/deviation.md) shows deviation from ANSI-Forth.
* [doc/Cfunc.md](doc/Cfunc.md) shows how to interface C code with SimForth.

## Features

SimForth is not a Forth conceived to be embedded in micro-controllers. It is a
[token threaded forth](https://www.bradrodriguez.com/papers/moving1.htm)
developed in C++ and token threaded forth makes the slower Forth interpreters
and C++ makes the binary bigger. SimForth has been made for offering GIS and
spreadsheet extensions (still in gestation) not to follow the Forth
standard. The core of SimForth is for the moment not complete and some words are
currently in development. As consequence, SimForth is not 100% compliant to
[ANSI-Forth 2012](https://forth-standard.org/standard/words).

I am a Forth noob, SimForth has its own personal touch concerning the
implementation of the interpreter, dictionary, input buffer, some features have
been discarded (source editor, user variable, vocabulary) and some design issues
are still present. Certainly, purists will not agree with my way of having
implemented it. See [deviations](doc/deviation.md).

Implemented in SimForth:
* Standalone Forth. See [sub-project](src/standalone/).
* Shared library for your C++ project desiring to embed a Forth interpreter.
* No Return-Stack manipulation but auxiliary stack manipulation (makes Forth
  less crashy).
* The Data-Stack is mixing integer and floatting-point values: no separated
  floating point stack, no floatting-point words. Standard words such addition
  or times will use floatting point when needed. I did not want floatting-point
  words since this increase the number of primitives and make the code less
  readable.
* Can import C code and interface C functions from external libraries. See [documentation](doc/Cfunc.md).
* Minimal Forth system Glossary [doc: ](doc/glossary.md), [code: ](core/System/Core.fth).
* Display colorful and human-friendly error messages (more than gforth).
* Display colorful dictionary. See [screenshot](doc/img/Dictionary.png).
* Basic colorful debugger. See [screenshot](doc/img/Debugger.png).
* Interactive mode, interpret files and C++ strings.
* Auto-completion of words in the interactive mode.
* History of commands in the interactive mode.
* (WIP) Basic GTK+ IDE (auto-completion, color, dictionary and stack display). See [screenshot](doc/img/IDE.png).
* Basic GTK+ spreadsheet standalone using SimForth instead of Visual Basic. See [sub-project](src/spreadsheet/).
* Basic self-tests made in Forth. See [code](core/SelfTests/tests-core.fth). Continuous integration scripts.
* Can execute basic Legacy OpenGL code. See [code](core/OpenGL/OpenGL.fth).
* Data-Stack mixing integer and float values (no separated floating point stack).
* Max number of characters for a word is 32. Unicode words can be accepted while this feature is not really tested.
* Save and load dump dictionary.

To be fixed / Work in progress:
* See [issue](https://github.com/Lecrapouille/SimForth/issues/1)
* Words to be implemented: locals, throw, catch, multi-tasking ...
* Forth structures are not compatible with C structure.
* Little/Big endian compatibility when dictionary is saved. For the moment only
AMD64-style architecture is managed.

Known issues:
* Disastrous dictionary byte manipulation.
* Dictionary having C functions loaded from shared libraries cannot be
  saved/loaded since pointers are not exported.

## Q/A

* Why SimForth? When I started to develop SimTaDyn I wanted a scripting language
  with GIS primitives for managing my maps. SimForth was originally used in my
  student project named
  [SimTaDyn](https://github.com/Lecrapouille/SimTaDyn/tree/release-EPITA-2004)
  which is a Geographic Information System (GIS) in which maps can be
  manipulated as a spreadsheet and where Forth replaces the Excel Visual
  Basic, the scripting language for formulas stored in spreadsheet cells. I am
  currently moving apart SimTaDyn code as modules and the Forth interpreter is
  one of these external parts. SimTaDyn is still a work in progress project but
  now, SimForth has its own git repo and I hope have it's own life and used for
  my other projects.

* Why not using Lua or Python or Julia? Because Forth code is compact, has no
  syntax (thanks to the reverse polish notation (RPN) and therefore, a Forth
  script is enough minimalist to be embedded in cells of spreadsheets in the
  same way of Excel Visual Basic. Minimalist does not mean weak functionalities:
  Forth is coming from embedded systems in 70's and has been used in astronomy,
  it has a very tiny footprint memory: up to 64 KiB for classic Forth. This size
  includes the interpreter and the virtual machine holding the byte code shared
  with the memory. In kind of way, a spreadsheet cell is kind of
  micro-controller: compactness is the key! This reduced size is possible mainly
  because the interpreter does not have to manage complex language syntax. In
  addition, this language can self-evolving its syntax (metaprogramming), living
  in a multi-tasking virtual machine that can easily be dumped in a file and
  shared. Forth is therefore more power than Excel Visual Basic. Lua or Python
  or Julia are a size more complex and heavy while offering a more friendly
  syntax and powerful libraries.

* Why developing another Forth interpreter? Why not simply using gforth? In
  2004, as a student, I wanted to learn Forth after my father gave me the book
  "FORTH" by W. P. Salman, O. Tisserand and B. Toulout, Editions Eyrolles, 1983.
  SimTaDyn started as a one-year student project and making a Forth interpreter
  was a good idea to impress my teachers (while, in fact, they were not all
  impressed by the beauty of Forth ... and by SimTaDyn). The other reason was
  the code source of gforth is too obscure for me, pforth can easily be crashed
  and I did not know at this time the uncrushable 4th. In addition, I wanted to
  develop my own Forth managing my desired features and, as a noob, to implement
  core features in C and interface them with Forth words. They usually do that
  in Python so why not in Forth!

* Why using C++ for developing Forth? Should not be better if written in
  assembly? Yes, assembly would have been a better choice but I wanted to have a
  portable Forth and use the inheritance of C++ to implement easily different
  Forth interpreters (from classic Forth to a specialized Forth for GIS).

* So why using SimForth? SimForth is the scripting language for SimTaDyn and
  should be used from SimTaDyn only since SimForth, used as standalone, is less
  powerful than any serious Forth interpreter and it is two times slower than
  pforth, five compared to gforth. See [benchmark](tests/bench).  In my opinion,
  SimForth used as shared library, offers a simpler API than gforth.
