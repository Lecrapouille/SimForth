# SimForth

[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://github.com/Lecrapouille/SimForth/blob/master/LICENSE)
[![codecov](https://codecov.io/gh/Lecrapouille/SimForth/branch/master/graph/badge.svg)](https://codecov.io/gh/Lecrapouille/SimForth)
[![coveralls](https://coveralls.io/repos/github/Lecrapouille/SimForth/badge.svg?branch=master)](https://coveralls.io/github/Lecrapouille/SimForth?branch=master)

|Branch     | **`Linux/Mac OS`** | **`Windows`** |
|-----------|------------------|-------------|
|master     |[![Build Status](https://travis-ci.org/Lecrapouille/SimForth.svg?branch=master)](https://travis-ci.org/Lecrapouille/SimForth)|[![Build status](https://ci.appveyor.com/api/projects/status/github/lecrapouille/SimForth?svg=true)](https://ci.appveyor.com/project/Lecrapouille/SimForth)|
|development|[![Build Status](https://travis-ci.org/Lecrapouille/SimForth.svg?branch=dev-refacto)](https://travis-ci.org/Lecrapouille/SimForth)||

SimForth is a Forth-like scripting language used for the
[SimTaDyn](https://github.com/Lecrapouille/SimTaDyn) project. SimTaDyn is a
work-in-progress Geographic Information System (GIS) that can be manipulated as
a spreadsheet and where Forth replaces Visual Basic (the scripting language used
in Excel-like spreadsheets).

SimForth has been developed in C++ and is a derived Forth interpreter for
offering (in the future) GIS extensions. It is not 100% compliant to
ANSI-Forth 2012. Nevertheless SimForth can be compiled as a basic [standalone
Forth
interpreter](https://github.com/Lecrapouille/SimForth/blob/master/doc/standalone.md)
in the same way than gforth, pforth and a shared library that can [be embedded in
a C/C++ project](https://github.com/Lecrapouille/LinkAgainstMyLibs/blob/master/Forth/src/main.cpp).

Note that in SimTaDyn, a Forth editor made in GTK+ is present but not offered
(yet) in this repo (SimForth is moving apart from the SimTaDyn git repo for
living in its own life in a separated git repo).

## Why SimForth?

* Why SimForth? When I started to develop SimTaDyn I wanted a scripting language
  with GIS primitives for managing my maps (note: GIS primitives is in
  gestation).

* Why not using Lua or Python or Julia? Because Forth has no syntax (thanks to
  the Reverse Polish notation, RPN) and therefore a Forth script is enough
  minimalist to be embedded in cells of spreadsheets in the same way of Excels'
  Basic. Minimalist does not mean no functionalities! Forth is coming from
  embedded systems (bare metal), it has a very tiny footprint memory up to 64
  KiB for classic Forth. This size includes the virtual machine, holding byte
  codes, and the interpreter. This reduced size is possible mainly because the
  interpreter does not have to manage complex syntax. In addition, this language
  can self-evolving its syntax, living in a multi-tasking virtual machine that
  can easily be dumped in a file and shared. Lua or Python or Julia are a size
  more complex while offering a more friendly syntax and powerful libraries.

* Why developing another Forth interpreter? Why not simply using gforth? Because
  gforth code source is too complex, and having a 100% compliant ANSI Forth does
  not interest me (for example the embedded source editor). The main reason is I
  wanted to develop my own Forth managing my desired features.

* Why using C++ for developing Forth? Should not be better if written in
  assembly? Yes, assembly would have been a better choice but I wanted to have a
  portable Forth and use the inheritance of C++ to implement easily different
  Forth interpreters (from classic Forth to a specialized Forth for GIS). I also
  want to create a library.

## Notes

**Note:** I am a beginner in Forth and SimForth is, at this stage, a basic Forth
interpreter does not have complete libraries and does not follow completely the
Forth 2012 standard. If you are looking for a faster and complete forth, you
should look at some projects:
* [gforth](https://gforth.org/),
* [pforth](http://www.softsynth.com/pforth/),
* [4th](https://thebeez.home.xs4all.nl/4tH/),
* [VFX](https://www.mpeforth.com/vfxlinux.htm)
* [reforth](https://github.com/seanpringle/reforth),
* [colorforth](http://www.figuk.plus.com/articles/chuck.pdf).

## Inspiration

The code source of SimForth has been largely inspired by the book and by projects:
* "FORTH" by W. P. Salman, O. Tisserand and B. Toulout, Editions EYROLLES, 1983
* [pforth](http://www.softsynth.com/pforth/),
* jonesforth [part1](https://github.com/AlexandreAbreu/jonesforth/blob/master/jonesforth.S)
and [part2](https://github.com/AlexandreAbreu/jonesforth/blob/master/jonesforth.f).
* colorforth [colors](http://www.profibing.de/colorforth/display.html).

## Installation

Read [docs](docs/standalone.md)

## Documentation

Newbie with Forth? DO NOT PANIC! Read [Starting FORTH](https://www.forth.com/starting-forth/) first.
More information inside the folder [docs](docs).

## Benchmark

SimForth is two times slower than pforth. See [benchmark](tests/bench).
