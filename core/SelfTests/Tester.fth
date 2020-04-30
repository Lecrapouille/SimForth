\ Inspired by http://lars.nocrew.org/forth2012/testsuite.html
\ and simplified for SimForth
\
\ This file creates words for unit testing Forth words

DECIMAL

\ -------------------------------------------------------------
\ Reserve an empty space inside the dictionary to store expected
\ results of the word to test. We name this space Expected-Stack.
CREATE   ACTUAL-RESULTS 32 CELLS ALLOT
\ Expected-Stack depth.
VARIABLE ACTUAL-DEPTH
\ Expected-Stack depth.
VARIABLE START-DEPTH

\ -------------------------------------------------------------
\ Empty the Expected-Stack: handles underflowed stack too.
: EMPTY-STACK   ( ... -- )
    DEPTH START-DEPTH @ < IF
        DEPTH START-DEPTH @ SWAP DO 0 LOOP
    ENDIF
    DEPTH START-DEPTH @ > IF
        DEPTH START-DEPTH @ DO DROP LOOP
    ENDIF
;

\ -------------------------------------------------------------
\ Display line corresponding to error
: DISP-LINE   ( adr u -- )
    TYPE                                  \ Display the message
    TERM.RESET.COLOR
    SOURCE TYPE                       \ Display the stream line
    CR
;

\ Display an error message
: ERROR   ( adr u -- )   DISP-LINE EMPTY-STACK ;

\ Define terminal colors
: RED      TERM.STYLE.BOLD TERM.FG.RED TERM.COLOR ;
: GREEN    TERM.STYLE.BOLD TERM.FG.GREEN TERM.COLOR ;
: BLUE     TERM.STYLE.BOLD TERM.FG.BLUE TERM.COLOR ;

\ Define messages
: CORRECT-RES   GREEN s" CORRECT "              DISP-LINE ;
: INCORRECT-RES   RED s" INCORRECT RESULT "         ERROR ;
: WRONG-NUMBER    RED s" WRONG NUMBER OF RESULTS "  ERROR ;

\ -------------------------------------------------------------
\ Set the Expected-Stack depth.
: T{    ( -- )    DEPTH START-DEPTH ! ;

\ -------------------------------------------------------------
\ Record depth and content of stack.
: ->    ( ... -- )
    DEPTH DUP ACTUAL-DEPTH !                     \ Record depth
    START-DEPTH @ > IF         \ If there is something on stack
        DEPTH START-DEPTH @ - 0 DO
            ACTUAL-RESULTS I CELLS + !              \ Save them
        LOOP
    ENDIF
;

\ -------------------------------------------------------------
\ Compare Expected-Stack contents with saved (actual) contents.
: }T    ( ... -- )
    DEPTH ACTUAL-DEPTH @ == IF                \ If depths match
        DEPTH START-DEPTH @ > IF \ If there is something on the
                                                       \ stack.
            DEPTH START-DEPTH @ - 0 DO    \ For each stack item
                ACTUAL-RESULTS I CELLS + @
                                 \ Compare actual with expected
                <> IF     \ Actual and expected values mismatch
                    INCORRECT-RES
                    2R> 2DROP RETURN          \ Clean Iterators
                ENDIF
            LOOP
            CORRECT-RES    \ Actual and expected values matched
        ENDIF
    ELSE                                       \ Depth mismatch
        WRONG-NUMBER
    ENDIF
;

\ -------------------------------------------------------------
\ Talking comment.
: TESTING   ( -- )
   BLUE
   SOURCE
   DUP >R                              \ Backup number of chars
   TYPE  TERM.RESET.COLOR CR
   R> >IN                          \ Skip chars of input stream 
;
