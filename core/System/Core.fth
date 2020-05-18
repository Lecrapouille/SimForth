\ =============================================================
\ Minimal Forth system.
\ Some code comes from the book FORTH by Salman, Tisserand and
\ Toulout. Other code comes from the code of Joneforth
\ https://github.com/nornagon/jonesforth
\ =============================================================

\ -------------------------------------------------------------
\ Word fixing compatibility between SimForth and ANSI-Forth
\
\ In SimForh, stack slots are named "Cells". They are an union
\ between an int64 and a double.
\
\ In SimForh, dictionary addresses are simply dictionary indices.
\ A dictionary slots are named "Tokens" (2 bytes). The SimForh
\ pointer (HERE) to the first empty dictionary slot cannot be
\ misaligned because we cannot move it to one byte.
\ -------------------------------------------------------------

: TOKENS ( n1 -- n2 )       TOKEN * ; \ 1 *
: CELLS  ( n1 -- n2 )       CELL  * ; \ 2 TOKENS
: CELL+  ( n1 -- n1+cell )  CELL  + ;

\ Convert dictionary address to byte address for BYTE@ and BYTE!
: >BYTES[]  ( token-addr -- byte-addr )   2 * ;

\ SimForth HERE cannot be misaligned because it cannot be moved
\ to MSB and LSB of a token. Nevertheless, we need to access to
\ bytes.
\
\ Align the number n1 to a multiple of tokens
\ Equivalent to modulo((n+1), 2) where 2 is the size in bytes of
\ a token.
: ALIGN  ( n1 -- n2 )    1+ -2 AND ;

\ Convert the number of chars to a number of tokens
: CHARS  ( n1 -- n2 )    ALIGN ;
: CHAR+  ( n1 -- n2 )    1+ CHARS ;

\ Reserve space for one character. This is not possible in SimForth
\ we have to reserve a token because this word displace HERE.
: C,  ( char -- ) , ;

\ Fetch a byte in the MSB of the token
: B_@  ( addr -- char )    >BYTES[] BYTE@ ;

\ Fetch a byte in the LSB of the token
: _B@  ( addr -- char )    >BYTES[] 1 + BYTE@ ;

\ Store a byte in the MSB of the token
: B_!  ( char addr -- )    >BYTES[] BYTE! ;

\ Store a byte in the LSB of the token
: _B!  ( char addr -- )    >BYTES[] 1 + BYTE! ;

\ Non implementable words
: C@ ABORT" C@ shall be replaced by BYTE@ and the address x2" ;
: C! ABORT" C! shall be replaced by BYTE! and the address x2" ;

\ Compile time tick
: [']   ( <name> -- xt )
   ?COMP ' POSTPONE LITERAL
; IMMEDIATE

\ Terminal style enums
: TERM.STYLE.NORMAL 0 ;
: TERM.STYLE.BOLD 1 ;
: TERM.STYLE.DIM 2 ;
: TERM.STYLE.ITALIC 3 ;
: TERM.STYLE.UNDERLINE 4 ;
: TERM.STYLE.BLINK 5 ;
: TERM.STYLE.RBLINK 6 ;
: TERM.STYLE.REVERSED 7 ;
: TERM.STYLE.CONCEAL 8 ;
: TERM.STYLE.CROSSED 9 ;

\ Terminal foreground color enums
: TERM.FG.BLACK 30 ;
: TERM.FG.RED 31 ;
: TERM.FG.GREEN 32 ;
: TERM.FG.YELLOW 33 ;
: TERM.FG.BLUE 34 ;
: TERM.FG.MAGENTA 35 ;
: TERM.FG.CYAN 36 ;
: TERM.FG.GRAY 37 ;
: TERM.FG.RESET 38 ;

\ Set terminal color to default state
: TERM.RESET.COLOR TERM.STYLE.NORMAL TERM.FG.RESET TERM.COLOR ;

\ Standard words for manipulating BASE.
: OCTAL   ( -- )   #8  BASE! ; \ Switch current base to base 8
: DECIMAL ( -- )   #10 BASE! ; \ Switch current base to base 10
: HEX     ( -- )   #16 BASE! ; \ Switch current base to base 16

\ Display the current base in decimal
: BASE?  ( -- )   BASE BASE DECIMAL . BASE! ;

\ -------------------------------------------------------------
\ Modules: INTERNAL ... code1 ... EXTERNAL ... code2 ... MODULE
\ Code between INTERNAL and external are private.
\ Code between EXTERNAL and module are public but refer private
\ words.
\
\ INTERNAL saves the Name Field Address (NFA) of the word right
\   before INTERNAL (therefore LATEST).
\ EXTERNAL saves the pointer to the word right after EXTERNAL
\   (therefore HERE).
\ MODULE takes the two NFA compute the relative address and
\   change the value of the Link Field Address (LFA) of the word
\   after EXTERNAL.In effect, it makes the dictionary skip over
\   all words in between INTERNAL and EXTERNAL.
\ -------------------------------------------------------------
: >LFA  ( nfa -- lfa )  >CFA TOKEN - ;     \ Convert NFA to LFA
: INTERNAL: ( -- nfa )  LATEST ; \ NFA of the word right before
: EXTERNAL: ( -- here ) HERE ;   \ NFA of the word right before
: MODULE    ( nfa here -- ) \ FIXME DUP >LFA TUCK - TOKEN! ;
   DUP >R                \ Backup HERE to be converted into LFA
   SWAP -                           \ HERE - NFA: relative jump
                           \ to the previous word in dictionary
   R> >LFA                                \ Convert HERE to LFA
   TOKEN!                          \ Store relative jump at LFA
;
\ TODO MODULE <name> -> call CREATE to avoid crapping dictionary

\ -------------------------------------------------------------
\ Dictionary
\ -------------------------------------------------------------

\ Returns the beginning address of a scratchpad area used to
\ hold character strings for intermediate processing.
: PAD  ( -- addr )   HERE #128 + ;

\ Fill with space characters at consecutive set of cells
: BLANKS  ( addr nb_cells -- )   $20 FILL ;

\ Fill with zeros at consecutive set of cells
: ERASE    ( addr nb_cells -- )   0 FILL ;

\ Create a buffer of n cells whose address is returned at run time.
: BUFFER: ( n "<name>" -- ; -- addr )
   CREATE ALLOT
;

\ Add n to the cell number refer by its address.
: +!  ( n addr -- )   SWAP OVER CELL@ + SWAP ! ;

\ -------------------------------------------------------------
\ if else then
\ -------------------------------------------------------------

: >MARK    ( -- addr )
   HERE              \ save location of the offset on the stack
   0 TOKEN,                            \ compile a dummy offset
;

: OFFSET    ( -- addr )
   HERE -               \ calculate the offset of the mark back
   TOKEN,                             \ compile the offset here
;

: IF
   ?COMP                     \ Abort if not in compilation mode
   COMPILE 0BRANCH
   >MARK
; IMMEDIATE

\ Alias name for THEN
: ENDIF
   HERE OVER -    \ calculate the offset from the address saved
                                                 \ on the stack
   SWAP TOKEN!   \ store the offset in the back-filled location
; IMMEDIATE

: ELSE
   COMPILE BRANCH
   >MARK SWAP
   [COMPILE] ENDIF
; IMMEDIATE

: THEN \ Compatibility with ansi Forth
   POSTPONE ENDIF
; IMMEDIATE

\ -------------------------------------------------------------
\ Loops
\ -------------------------------------------------------------

\ Mark the start of an indefinite loop. Usage:
\ BEGIN ... flag UNTIL
\ BEGIN ... flag WHILE ... REPEAT
\ BEGIN ... AGAIN
: BEGIN   ( -- )
   HERE                            \ save location on the stack
; IMMEDIATE

\ If flag is false, go back to BEGIN.
\ If flag is true, terminate the loop.
: UNTIL   ( flag -- )
   COMPILE 0BRANCH
   OFFSET
; IMMEDIATE

\ Go back to BEGIN (infinite loop).
: AGAIN   ( -- )
   COMPILE BRANCH
   OFFSET
; IMMEDIATE

\ If flag is true, continue.
\ If flag is false, terminate the loop (after REPEAT).
: WHILE   ( flag -- )
   COMPILE 0BRANCH
   >MARK
; IMMEDIATE

\ Resolves forward branch from WHILE; goes back to BEGIN.
: REPEAT   ( -- )
   COMPILE BRANCH
   SWAP                  \ get the original offset (from BEGIN)
   OFFSET                         \ and compile it after BRANCH
   DUP
   HERE SWAP -                          \ calculate the offset2
   SWAP TOKEN!      \ and back-fill it in the original location
; IMMEDIATE

: UNLESS
   COMPILE 0 ==             \ compile NOT (to reverse the test)
   [COMPILE] IF             \ continue by calling the normal IF
; IMMEDIATE

\ Sets up a finite loop, given the index and limit. Usage:
\ DO ... LOOP
\ DO ... n +LOOP
: DO   ( limit index -- )
   COMPILE 2>R
   HERE
; IMMEDIATE

\ Add one to the loop index. If the loop index is then equal to
\ the loop limit, discard the loop parameters and continue execution
\ immediately following LOOP. Otherwise continue execution at the
\ beginning of the loop (after DO).
: LOOP   ( -- )
   COMPILE (LOOP?)
   COMPILE 0BRANCH
   OFFSET
   COMPILE 2RDROP
; IMMEDIATE

\ TODO +LOOP

\ Terminate the loop immediately.
: LEAVE   ( -- )
   2R> >R >R
;

\ -------------------------------------------------------------
\ Switch case
\ -------------------------------------------------------------

: CASE 0 ; IMMEDIATE  \ push 0 to mark the bottom of the stack

: OF
   COMPILE OVER
   COMPILE ==
   [COMPILE] IF
   COMPILE DROP
; IMMEDIATE

\ ENDOF is the same as ELSE
: ENDOF
   [COMPILE] ELSE
; IMMEDIATE

: ENDCASE
   COMPILE DROP
   \ keep compiling THEN until we get to our zero marker
   BEGIN
      ?DUP
   WHILE
      [COMPILE] THEN
   REPEAT
; IMMEDIATE

\ -------------------------------------------------------------
\ IO
\ -------------------------------------------------------------

\ Display the space character
: BL  ( n -- ) $20 EMIT ;

\ Display n times the space character
: SPACES  ( n -- ) 0 DO BL LOOP ;

\ Receives u1 characters (or until carriage return) from the
\ terminal keyboard and stores them, starting at the address.
\ The count of received characters is returned.
: ACCEPT  ( addr u1 -- u2 ) \ : EXPECT 0 DO KEY LOOP ;
   0 DO
      DUP                         ( addr addr -- )
      I CELLS +                ( addr addr+I -- )
      KEY DUP >R SWAP !    ( store stdin at addr+I and backup stdin )
      R> $d == IF             ( abort if stdin == '\n' )
         LEAVE
      ENDIF
   LOOP
   DROP
   I? 1+
;

\ -------------------------------------------------------------
\ Constants and arithmetic functions
\ -------------------------------------------------------------

:  PI  3.141592653 ;                                \ PI number
: -PI -3.141592653 ;                       \ Negative PI number
: E  2.71828 ;                  \ base of the natural logarithm
: FALSE   0 ;                             \ Return a false flag
: TRUE   -1 ;                              \ Return a true flag

\ Use for linear algebra (absorbing and neutral)
: ZERO    0 ;
: ONE     1 ;

\ flag is true if and only if n is equal to zero.
: ZERO=    ( n -- flag )   ZERO == ;

\ flag is true if and only if n is not equal to zero.
: ZERO<>   ( n -- flag )   ZERO <> ;

\ flag is true if and only if n is greater than zero.
: ZERO>    ( n -- flag )   ZERO > ;

\ flag is true if and only if n is less than zero.
: ZERO<    ( n -- flag )   ZERO < ;

\ logical negation
: NOT   ( n -- !n )   ZERO == ;

\ Invert all bits of n, giving its logical inverse.
: INVERT   ( n -- 1'comp )   -1 XOR ;

\  Negate n, giving its arithmetic inverse.
: NEGATE   ( n -- -n )   0 SWAP - ;

\ Return the shifting of n by one bit toward the most
\ significant bit, filling the vacated least-significant
\ bit with zero.
: 2*  ( n -- n*2 )   1 LSHIFT ;

\ Return the shifting of n by one bit toward the least
\ significant bit, leaving the most-significant bit
\ unchanged.
: 2/  ( n -- n/2 )   1 RSHIFT ;

\ Return the absolute of the number (float or int)
: ABS    ( n -- |n| )
   DUP 0<
   IF NEGATE ENDIF
;

\ Return the squared value of the number (float or int)
: SQUARED    ( n -- n*n )
   DUP *
;

\ Return the max value of two numbers (float or/and int)
: MAX  ( n1 n2 -- n1|n2 )
   2DUP < IF
      SWAP
   ENDIF
   DROP
;

\ Return the max value of two numbers (float or/and int)
: MIN  ( n1 n2 -- n1|n2 )
   2DUP > IF
      SWAP
   ENDIF
   DROP
;

\
: WITHIN   ( test low high -- flag )
  >R OVER < 0=                                     \ test flag1
  SWAP R> <                                       \ flag1 flag2
  AND
;

\ -------------------------------------------------------------
\ Stack manipulation
\ -------------------------------------------------------------

: TUCK  ( x1 x2 -- x2 x1 x2 )   SWAP OVER ;

\ -------------------------------------------------------------
\
\ -------------------------------------------------------------

: ON    ( addr -- set true )
   TRUE SWAP !
;
: OFF    ( addr -- set false )
   FALSE SWAP !
;

\ -------------------------------------------------------------
\ Variables and constants
\ -------------------------------------------------------------

\ Variable:
\   VARIABLE foo               \ Create a variable with value 0
\   12 foo !                              \ Affect the value 12
\   foo @                                    \ Return its value
\   foo ?                                      \ Show its value
: VARIABLE CREATE CELL ALLOT ;

\ Value is a variable with the syntax for constants. The code
\ is shorter:
\   12 VALUE foo             \ Create a value with the value 12
\   foo .                                      \ Show its value
\   42 TO foo                             \ Affect the value 42
: VALUE      <BUILDS HERE ! CELL ALLOT DOES> CELL@ ;
: FVALUE      <BUILDS HERE ! CELL ALLOT DOES> FLOAT@ ; \ FIXME

\ In compilation mode creates a pointer to the value instead of
\ searching the value which is more time consuming.
: TO
   FIND 0= IF ABORT" Unknown word" ENDIF \ From next word in TIB
   >CFA 4 TOKENS +                          \ ... get its CFA+4
   STATE IF                                  \ Compilation mode
      ['] (TOKEN) TOKEN,                        \ Compile (TOKEN)
      TOKEN,                 \ Compile the address of the value
      ['] CELL! TOKEN,                                \ Compile !
   ELSE
      CELL!		               \ Update it straightaway
   THEN
; IMMEDIATE

\ Constant:
\   12 CONSTANT foo
\   foo .
: CONSTANT   <BUILDS CELL ALLOT DOES> CELL@ ;
: FCONSTANT   <BUILDS CELL ALLOT DOES> FLOAT@ ; \ FIXME

\ http://amforth.sourceforge.net/TG/recipes/Builds.html
\ <BUILDS is the older sibling of create. Unlike create it does
\ not add an execution token. Thus the word list entry created
\ is unfinished and calling it will crash the system.
\ : <BUILDS CREATE ;

\ Fetches the integer at an address and prints it.
: ?  ( addr -- )     CELL@ . ;

\ Fetches the float at an address and prints it.
: F?  ( addr -- )   FLOAT@ . ; \ FIXME

\ -------------------------------------------------------------
\ Defer is a value but holding an execution token
\ DEFER xt
\ ' + IS xt
\ -------------------------------------------------------------
: DEFER    <BUILDS TOKEN ALLOT DOES> TOKEN@ EXECUTE ;
: IS
   FIND 0= IF ABORT" Unknown word" ENDIF \ From next word in TIB
   >CFA 4 TOKENS +                          \ ... get its CFA+4
   STATE IF                                  \ Compilation mode
      ['] (TOKEN) TOKEN,                        \ Compile (TOKEN)
      TOKEN,                 \ Compile the address of the value
      ['] TOKEN! TOKEN,                               \ Compile !
   ELSE
      TOKEN!		               \ Update it straightaway
   THEN
; IMMEDIATE

\ -------------------------------------------------------------
\ Array
\ Creation: 10 ARRAY table
\ Store value 42 at index 5: 42 6 table !
\ Read the value of the index 5: 6 table @
\ -------------------------------------------------------------
: CHEK-INDEX ( val index array -- )
   DUP >R                         \ Backup the array address
   @ OVER <= IF ABORT" Index out of range" ENDIF
   DUP 0< IF ABORT" Negative index!" ENDIF
   R>
;

: ARRAY
   <BUILDS
      DEPTH 0= IF ABORT" No array size supplied!" ENDIF
      DUP 0< IF ABORT" Negative array size!" ENDIF
      DUP TOKEN,                  \ Store the size of the array
      CELLS ALLOT          \ Create the N elements of the array
   DOES> ( val index array -- val array+index )
      CHEK-INDEX
      SWAP 1+ CELLS +
;

HIDE CHEK-INDEX

\ -------------------------------------------------------------
\ Structure (TODO WIP !!!!!)
\
\ Example:
\ STRUCT
\   CELL FIELD .real
\   CELL FIELD .img
\ END-STRUCT complex
\
\ complex nb
\
\ 65535 nb .real !
\ 65534 nb .img !
\ -------------------------------------------------------------
VARIABLE TOTAL.TOKENS       1 TOTAL.TOKENS !
VARIABLE CURRENT.RECORD     0 CURRENT.RECORD !
VARIABLE BIND.NOW       FALSE BIND.NOW !

: FIELD
   <BUILDS
      TOTAL.TOKENS @ ,
      TOTAL.TOKENS +!
      IMMEDIATE
   DOES>
      CURRENT.RECORD @ ?DUP
      IF SWAP ENDIF
      @ +
      STATE BIND.NOW @ AND
      IF [COMPILE] LITERAL ENDIF
      FALSE BIND.NOW !
;

: MAKE.INSTANCE .s
   DUP ,
   DUP ." ALLOCATE " . CR
   CREATE ALLOT IMMEDIATE
;

: STRUCT ;

: END-STRUCT
   <BUILDS
      TOTAL.TOKENS @ ,
      1 TOTAL.TOKENS !
   DOES>
      @ MAKE.INSTANCE
;

: BIND TRUE BIND.NOW ! ; IMMEDIATE

\ : USE ' >CFA @ ;

HIDE TOTAL.TOKENS
HIDE CURRENT.RECORD
HIDE BIND.NOW
HIDE MAKE.INSTANCE

\ -------------------------------------------------------------
\ Hide definitions that the user should not use
\ -------------------------------------------------------------

HIDE >MARK
HIDE (TOKEN)
HIDE (LOOP?)
