\ =============================================================
\ Minimal Forth system.
\ Some code comes from the book FORTH by Salman, Tisserand and
\ Toulout. Other code comes from the code of Joneforth
\ https://github.com/nornagon/jonesforth
\ =============================================================

: [WORDS] WORDS ; IMMEDIATE

\ -------------------------------------------------------------
\ Terminal color
\ -------------------------------------------------------------
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

: TERM.FG.BLACK 30 ;
: TERM.FG.RED 31 ;
: TERM.FG.GREEN 32 ;
: TERM.FG.YELLOW 33 ;
: TERM.FG.BLUE 34 ;
: TERM.FG.MAGENTA 35 ;
: TERM.FG.CYAN 36 ;
: TERM.FG.GRAY 37 ;
: TERM.FG.RESET 38 ;

: TERM.RESET.COLOR TERM.STYLE.NORMAL TERM.FG.RESET TERM.COLOR ;

\ -------------------------------------------------------------
\ Compatibility with ANSI-Forth
\ -------------------------------------------------------------

: CELLS CELL * ;
: CELL+  ( n -- n+cell )   CELL + ;
: CELL-  ( n -- n+cell )   CELL - ;
: CELL*  ( n -- n*cell )   CELLS ;

\ -------------------------------------------------------------
\ Standard words for manipulating BASE.
\ -------------------------------------------------------------

: OCTAL   ( -- )   #8  BASE! ;  \ Switch current base to base 8
: DECIMAL ( -- )   #10 BASE! ; \ Switch current base to base 10
: HEX     ( -- )   #16 BASE! ; \ Switch current base to base 16

\ Display the current base in decimal
: BASE?  ( -- )   BASE BASE DECIMAL . BASE! ;

\ -------------------------------------------------------------
\ Constants
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
: 0=    ( n -- flag )   ZERO == ;

\ -------------------------------------------------------------
\ Stack manipulation
\ -------------------------------------------------------------

: TUCK  ( x1 x2 -- x2 x1 x2 )   SWAP OVER ;

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

\ Add n to the cell number refer by its address.
: +!  ( n addr -- )   SWAP OVER @ + SWAP ! ;

\ -------------------------------------------------------------
\
\ -------------------------------------------------------------

(
\ Compile time tick
: [']   ( <name> -- xt )
   ?COMP ' [COMPILE] LITERAL
; IMMEDIATE
)

\ -------------------------------------------------------------
\ if else then
\ -------------------------------------------------------------

: >MARK    ( -- addr )
   HERE              \ save location of the offset on the stack
   0 X,                                \ compile a dummy offset
;

: OFFSET    ( -- addr )
   HERE -               \ calculate the offset of the mark back
   X,                                 \ compile the offset here
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
   SWAP X!       \ store the offset in the back-filled location
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
   SWAP X!          \ and back-fill it in the original location
; IMMEDIATE

: UNLESS
   COMPILE 0=               \ compile NOT (to reverse the test)
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
: SPACE  ( n -- ) $20 EMIT ;

\ Display n times the space character
: SPACES  ( n -- ) 0 DO SPACE LOOP ;

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

\ : [CHAR]  ( <char> -- char , for compile mode )
\   CHAR [COMPILE] LITERAL
\ ; IMMEDIATE

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
\ Arithmetic
\ -------------------------------------------------------------

\ flag is true if and only if n is not equal to zero.
: 0<>   ( n -- flag )   ZERO <> ;

\ flag is true if and only if n is greater than zero.
: 0>    ( n -- flag )   ZERO > ;

\ flag is true if and only if n is less than zero.
: 0<    ( n -- flag )   ZERO < ;

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

: ABS    ( n -- |n| )
   DUP 0<
   IF NEGATE ENDIF
;

: SQUARED    ( n -- n*n )
   DUP *
;

: MAX  ( n1 n2 -- n1|n2 )
   2DUP < IF
      SWAP
   ENDIF
   DROP
;

: MIN  ( n1 n2 -- n1|n2 )
   2DUP > IF
      SWAP
   ENDIF
   DROP
;

\ -------------------------------------------------------------
\ Variables and constants
\ -------------------------------------------------------------

\ Variable:
\   VARIABLE foo              \ Create a variable uninitialized
\   12 foo !                           \ Affect it the value 12
\   foo ?                                   \ Show the value 12
: VARIABLE CREATE CELL ALLOT ;

\ Value: shorter code for creating + initializing a variable
\   12 VALUE foo
\   foo ?
: VALUE    CREATE HERE ! CELL ALLOT ;

\ Constant:
\   12 CONSTANT foo
\   foo .
\ TODO : CONSTANT <BUILDS , DOES> @ ;

\ TODO http://amforth.sourceforge.net/TG/recipes/Builds.html
\ <BUILDS is the older sibling of create. Unlike create it does
\ not add an execution token. Thus the word list entry created
\ is unfinished and calling it will crash the system.
\ : <BUILDS CREATE ;

\ Fetches the integer at an address and prints it.
: ?  ( addr -- )     @ . ;

\ Fetches the float at an address and prints it.
: F?  ( addr -- )   F@ . ;

\ -------------------------------------------------------------
\ Floating point operations
\ -------------------------------------------------------------

: FNEGATE -1.0 * ;
: >INT ZERO + ;
: >FLOAT 0.0 F+ ;

\ -------------------------------------------------------------
\ Hide definitions that the user should not use
\ -------------------------------------------------------------

SMUDGE >MARK
