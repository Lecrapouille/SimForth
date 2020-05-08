# SimForth deviation

**Work in progress**

SimForth is a Forth-like interpreter and therefore it does follow 100% of
ANSI-Forth 2012. Main differences are explained in this document. Note I am a
beginner in Forth and not an expert, as consequence some of my decision may be
an odd decision to Forth expert's eyes.

## Reminder and definition

A `Cell` is a Forthism referring to the space a number takes up. Because Forth
is based on the Reverse Polish notation (RPN), parameters of word (Forthism
referring to functions) are stored in a stack (named by convention Data Stack or
Parameter Stack). Classic Forth uses a single data stack holding 16-bits
integers.

A `Token` is the type of the byte code elements stored in the dictionary. In
SimForth, such as classic Forth, tokens are unsigned 16-bit integers. No 8-bits
cells are managed. As consequence string shall be aligned to token rooms.

A Forth dictionary holds byte codes which are identifiers to compiled Forth
words. The dictionary contains up to 65536 tokens (because tokens are 16-bits: 2^16 =
65536).

Dictionary is made of two parts which can be separated or mixed : the byte
code and an index for searching word definitions (word entries). Following Forth
78 convention word entries and definitions are consecutive in the dictionary. A
word entry is made of:
- the name of the word (up to 32 chars).
- 1 byte for storing flags (immediate word, smudge word) and the number of chars
  used in the name.
- Link Field Address (LFA): is a address (relative or not) to the previous word.
- Code Field Address (CFA): holding the token referring to the word.
- Parameter Field Address (PFA): a list of tokens referring to compiled Forth words.

## Deviation

### Internal Structure

SimForth does not use C pointers to dive inside the dictionary. Addresses are
simply indices. In SimForth tokens are 16-bits and the dictionary maximum size
is 2^16 bits. The advantage is you cannot leave the dictionary space by
error. The inconvenient is that you cannot access to external elements of the
dictionary such at the data stack or the terminal input buffer. This is fine for
a non bare metal environment.

SimForth dictionary follows the Forth 78 convention and does not use modern C++
structure such as hash table or an separated C link list using C++ pointers ?
Simply for easily dumping the whole dictionary into a single binary file with no
parsing and trivial concatenation of dictionaries. To follow this idea: LFA in
SimForth are relative address to the previous word. This makes easy translating
word entries.

Classic Forth cells are 16-bits integers and Forth have to manipulate double
cells in the data stack to have 32-bits integers. Some extension written in
Forth implement a float data stack (when needed). Like 4th, SimForth does not
manage 8-bit cells or 16-bit double-length cell and does not offer natively
operators such `D+` ... In SimForth a cell is the union between a 64-bit integer
and a 64-bit float. As consequence SimForth does not need an extra float
stack. SimForth data stack mixes float and int. Arithmetic operators force the
type of the cell. For example `2.5 1 +` will force the result to be an integer
(4) while `2.5 1 f+` will force the result to be a float (3.5). Roundness is
managed: `2.4 1 +` will return the integer 3.

**Note: float operators will be removed.** `2.5 1 +` will give 3.5 and for casting
to integer you will have to use words `>INT` and `>FLOAT`.

64-bits allows to hold pointers to C functions that may be used when interfacing
external C functions in SimForth.

### Numbers are not words

In ANSI forth you can replace a number by a word. This is not possible in
SimForth for security reasons.  Look: `: 42 66 ; 42` in ANSI-Forth will produce
`66` while in SimForth will still produce `42` because numbers are checked
before words. Note: no warnings are produced in SimForth for the moment.

### No User Variables

In the goal to reduce crash, no `user variables` are used but directly C++ class
manage them.

* `BASE` is not a user variable but a word. You have to call `BASE` and `BASE!`
  instead of `BASE @` and `BASE !`. The setter checks the value of the base and
  throw and exception if the value is odd.

* `HERE` is read-only to avoid changing it risking to erase the dictionary with
  code like `0 HERE @ 65535 BLANK`.  Operators such as `ALLOT`, already displace
  it.

* `S0` is not implemented because the user does not have to play with it.

* `STATE` is not implemented. Use `COMP?` to throw an error if the interpreter
  is not in compilation mode. In Forth 0 means the interpreter is executing code
  and non-zero means compiling a word.  In SimForth 0 means the interpreter is
  executing code, 1 means compiling a word and 2 means the interpreter is
  skipping comments (but you are not supposed to observe this state since your
  are in comments). Why this extra mode? While I appreciate the beauty of Forth
  defining comments: `: ( ')' WORD DROP ; IMMEDIATE` I am not a bug fan to let
  the interpreter Because I do not like defining

### Recursivity

**Note: this is a bad choice. The standard behavior will be restored**.

In ANSI-Forth we have to use words `RECURSE` or `RECURSIVE` to refer to the word
currently compiling. In SimForth has the word `RECURSE` for compatibility but
you can simply use the name of the word currently compiling.

```
: FIBONACCI ( N1 -- N2 )
    DUP 2 < IF DROP 1 EXIT THEN
    DUP  1 - FIBONACCI
    SWAP 2 - FIBONACCI  + ;
20 FIBONACCI
```

### No Terminal Input Buffer

In SimForth, the Terminal Input Buffer (TIB) does not exist in sense of Classic
Forth. In SimForth TIB has been replaced by C++ class Stream splitting words
from either a file, a string or the interactive input. Because addresses inside
the dictionary cannot refer to external C++ pointers and therefore cannot refer
to the TIB. As consequence SimForth words for I/O are not the same than
standard.

### Reduced dictionary entry manipulation

* Words such as `FIND`, `>BODY` are powerful but crash prone. They are privately
  implemented in C++.

* `FORGET` is not implemented because you can easily erase the whole dictionary.

* In classic Forth, words are smudged while the `;` is not reached and the
  definition valid. In SimForth, odd definitions are simply discarded and the
  `SMUDGE` word is only use to make invisible the desired word to dictionary
  lookup searches (while still traversed). This make words private and no longer
  compilable into new definitions while still functional when for older words.

`WORDS` display a colorful dump of the dictionary instead of display an ugly
list of words. I'm using it to help me debugging. See the document standalone.md
for more information.

### Nesting comments

I never like that `(` ends to the first `)`. I decided to allow nesting
comments. You can nest comments as long as they are well balanced. In SimForth
comments are directly managed inside the C++ code since TIB operations are not
allowed.

### No Byte manipulation

Comes from a Forth noob decision of mine. In SimForth storing bytes is not
allowed while possible in classic Forth. Words such as `ALLOT` and `,` modifies
the value of the word HERE. In SimForth Dictionary rooms are tokens (two bytes)
and I did not to reduce my dictionary to 64 KiB but to 65536 tokens. In addition
I did not want to manage the alignment of the word HERE (undefined behavior in
classic Forth) as consequence in SimForth the value of HERE should be divided by
2 witch is not possible with integers.
