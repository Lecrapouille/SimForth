# SimForth Words Glossary

Work in progress. See https://forth-standard.org/standard/words for more details

## Primitives

### Core

* NOP
* BYE
* SEE
* WORDS
* ABORT
* PABORT_MSG
* ABORT_MSG
* SET_BASE
* GET_BASE

### Input

* SOURCE
* KEY
* TERMINAL_COLOR
* WORD
* TYPE
* TO_IN

### Code

* EVALUATE
* TRACES_ON
* TRACES_OFF

### Display

* EMIT
* CR
* DOT_DSTACK
* DOT
* DOT_STRING

### String

* STORE_STRING
* `s"` (token SSTRING)
* `z"` (token ZSTRING) On interpration mode, create a zero-string compatible with C char* and return the C-compatible address. Beware, and contrary to `s"` which returns the string size and the indice of the dictionnary, `z"` return a
C pointer which cannot be used by other Forth words.
* MATCH
* SPLIT

### Interfaces with C libraries

* TO_C_PTR
* CLIB_BEGIN
* CLIB_END
* CLIB_ADD_LIB
* CLIB_C_FUN
* CLIB_C_CODE
* CLIB_EXEC

### System

* FORK
* SELF
* SYSTEM

### Branching

* INCLUDE
* BRANCH
* ZERO_BRANCH
* QI
* I
* QJ
* J

### Secondary word creation

* COMPILE_ONLY
* STATE
* NONAME
* COLON
* SEMI_COLON
* EXIT
* RECURSE
* LITERAL
* PCREATE
* CREATE
* BUILDS
* PDOES
* DOES
* IMMEDIATE
* HIDE
* TICK
* COMPILE
* ICOMPILE
* POSTPONE
* EXECUTE
* LEFT_BRACKET
* RIGHT_BRACKET

### Dictionary manipulation

* TOKEN
* CELL
* HERE
* LATEST
* TO_CFA
* FIND
* FILL
* CELLS_MOVE
* BYTE_FETCH
* BYTE_STORE
* TOKEN_COMMA
* TOKEN_FETCH
* TOKEN_STORE
* CELL_COMMA
* ALLOT
* FLOAT_FETCH
* CELL_FETCH
* CELL_STORE

### Auxiliary stack manipulation

* TWOTO_ASTACK
* TWOFROM_ASTACK
* TO_ASTACK
* FROM_ASTACK
* DUP_ASTACK
* DROP_ASTACK
* TWO_DROP_ASTACK

### Floating point operations

* FLOOR
* ROUND
* CEIL
* SQRT
* EXP
* LN
* LOG
* ASIN
* ACOS
* ATAN
* SIN
* COS
* TAN,
* EQ_ZERO
* NE_ZERO
* GREATER_ZERO
* LOWER_ZERO

### Data stack manipulation

* TO_INT
* TO_FLOAT
* DEPTH
* PLUS_ONE
* MINUS_ONE
* LSHIFT
* RSHIFT
* XOR
* OR
* AND
* ADD
* MINUS
* TIMES
* DIVIDE
* GREATER
* GREATER_EQUAL
* LOWER
* LOWER_EQUAL
* EQUAL
* NOT_EQUAL
* TWO_SWAP
* TWO_OVER
* TWO_DROP
* TWO_DUP
* NIP
* ROLL
* PICK
* SWAP
* OVER
* ROT
* DROP
* DUP
* QDUP

### Comments

* LPARENT
* RPARENT
* COMMENT
* COMMENT_EOF

### Extension

* TOKENS
* CELLS
* CELL+
* >BYTES[]
* ALIGN
* CHARS
* CHAR+
* C,
* B_@
* _B@
* B_!
* _B!
* [']
* OCTAL
* DECIMAL
* HEX
* BASE?
* >LFA
* INTERNAL:
* EXTERNAL:
* MODULE
* PAD
* BLANKS
* ERASE
* BUFFER:
* +!
* IF
* ENDIF
* ELSE
* THEN
* BEGIN
* UNTIL
* AGAIN
* WHILE
* REPEAT
* UNLESS
* DO
* LOOP
* LEAVE
* CASE
* OF
* ENDOF
* ENDCASE
* BL
* SPACES
* ACCEPT
*  PI
* -PI
* E
* FALSE 
* TRUE
* ZERO
* ONE
* ZERO=
* ZERO<>
* ZERO>
* ZERO<
* NOT
* INVERT
* NEGATE
* 2*
* 2/
* ABS
* SQUARED
* MAX
* MIN
* WITHIN
* TUCK
* ON
* OFF
* VARIABLE
* VALUE
* FVALUE
* TO
* CONSTANT
* FCONSTANT
* ?
* F?
* DEFER
* IS
* ARRAY
* FIELD
* MAKE.INSTANCE
* STRUCT
* END-STRUCT
