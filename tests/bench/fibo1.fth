\ Fibonacci
: FIB1 ( N1 -- N2 )
    DUP 2 < IF DROP 1 EXIT THEN
    DUP  1 - RECURSE
    SWAP 2 - RECURSE  + ;

: FIB1-BENCH 100 0 DO
    30 0 DO I FIB1 DROP LOOP
  LOOP ;

FIB1-BENCH
