# SimForth Standalone

SimForth is compiled as:
* a standalone Forth interpreter in the same way than gforth, pforth
* and a shared library to be embedded in your C/C++ project if you desire
to have a Reverse Polish Notation script language.

This document will focus on the first point.

## SimForth Standalone Usage

```sh
cd SimForth
./build/SimForth -h

Usage:   ./build/SimForth [-option] [argument]
option:  -h              Show this usage
         -u              Show this usage
         -l dico         Load a SimForth dictionary file and smash the current dictionary
         -a dico         load a SimForth dictionary file and append to the current dictionary
         -s dico         Dump the current dictionary into a binary file
         -f file         Interprete a SimForth script file (ascii)
         -e string       Interprete a SimForth script string (ascii)
         -d              Pretty print the dictionary with or without color (depending on option -x)
         -p path         Append new pathes to look for file. Pathes are separated by character ':'
         -r path         Replace pathes to look for file. Pathes are separated by character ':'
         -i              Interactive mode. Type BYE to leave
         -x              Do not use color when displaying dictionary
```

More explanations on arguments (which can be combined):
* SimForth can interpret a script Forth passed either as a file (-f) or as a command line (-e).
* SimForth has an interactive mode (-i). Type `BYE` or `bye` to leave.
* In SimTaDyn, the SimForth editor is a GUI made in GTK+ but this not yet the case for this repo.
* SimForth dictionary can be displayed on the console with colors (-d) This is inspired by Charles Moore's ColorForth.
* A dictionary file is a binary file containing the "dump" of the Forth dictionary (-s).
* Dictionaries can be loaded (-l) or concatenated to an existing one (-a).

### Basic Example

```sh
./build/SimForth -e "1 2 + ."
```

Execute a simple addition and show its result on the console.

### Complex Example

```sh
./build/SimForth -f core/Tester.fth -e "T{ 1 2 3 -> 1 2 3 }T" -p
```

`-f core/tester.fth` loads a file allowing to unit test the system; `-e "T{ 1 2
3 -> 1 2 3 }T"` execute a unit test checking if storing 3 numbers on the data
stack and calling no functions undamaged the data stack.  Finally `-p` display
the dictionary (modern display of the word `WORDS`).

## Interactive mode

Words can be completed with the <TAB> key. Commands are saved in history in
`~/.SimForth/history.txt`.

## Looking for files

I you have typed `sudo make install` you will see in folders `/usr/share/SimForth/<version>/core`.
SimForth already knows this path but if you
want to add extra folders in which to search your files, you have to use the
option `-p path1:path2:...`. To replace the path use the option `-r
path1:path2:...`. The `:` is used for separating folders in the same way than the
Unix environment variable `$PATH` (the order of search is given by the order of
folders).

For example: `-p "/home/Me/scripts/core:/home/Me/more/scripts/" will
give `'.:/home/xxx/MyGitHub/SimForth/core:/usr/share/SimForth/0.1/data/core:/home/Me/scripts/core:/home/Me/more/scripts`
while `-r "/home/Me/scripts/core:/home/Me/more/scripts/" for replacing will give
`'.:/home/Me/scripts/core:/home/Me/more/scripts`.

Example:
```sh
ls /home/Me/scripts/
f1.fth f2.fth

./build/SimForth -p "/home/Me/scripts/" -f f1.fth -f f2.fth
```

**Work In Progress:** An environment variable SIMFORTHPATH will be used.
**Work In Progress:** A folder path in `~/.SimForth` will be used.
