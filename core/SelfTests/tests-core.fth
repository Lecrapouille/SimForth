\ Inspired from https://github.com/gerryjackson/forth2012-test-suite
\ Adapted for SimForth

\ INCLUDE core/SelfTests/Tester.fth \ FIXME search localy

DECIMAL

\ -------------------------------------------------------------
TESTING Check behavior of the tester (clean slate ?)
T{ -> }T
T{  1 ->  1 }T   T{  1  2 ->  1  2 }T   T{  1  2  3 ->  1  2  3 }T
T{ -1 -> -1 }T   T{ -1 -2 -> -1 -2 }T   T{ -1 -2 -3 -> -1 -2 -3 }T
\ T{  1.0 ->  1.0 }T   T{  1.0  2.0 ->  1.0  2.0 }T   T{  1.0  2.0  3.0 ->  1.0  2.0  3.0 }T

\ -------------------------------------------------------------
( TESTING Float-Int convertions
T{  1.1 >INT ->  1 }T    T{ 1.4 >INT ->  1 }T   T{  1.5 >INT ->  2 }T
T{ -1.1 >INT -> -1 }T   T{ -1.4 >INT -> -1 }T   T{ -1.5 >INT -> -2 }T
T{ 1 >FLOAT -> 1.0 }T
)

\ -------------------------------------------------------------
TESTING Integer arithmetic
T{ 0 5 + -> 5 }T
T{ 5 0 + -> 5 }T
T{ 0 -5 + -> -5 }T
T{ -5 0 + -> -5 }T
T{ 1 2 + -> 3 }T
T{ 1 -2 + -> -1 }T
T{ -1 2 + -> 1 }T
T{ -1 -2 + -> -3 }T
T{ -1 1 + -> 0 }T
T{ 0 5 - -> -5 }T
T{ 5 0 - -> 5 }T
T{ 0 -5 - -> 5 }T
T{ -5 0 - -> -5 }T
T{ 1 2 - -> -1 }T
T{ 1 -2 - -> 3 }T
T{ -1 2 - -> -3 }T
T{ -1 -2 - -> 1 }T
T{ 0 1 - -> -1 }T
T{ 0 1+ -> 1 }T
T{ -1 1+ -> 0 }T
T{ 1 1+ -> 2 }T
T{ 2 1- -> 1 }T
T{ 1 1- -> 0 }T
T{ 0 1- -> -1 }T

\ -------------------------------------------------------------
( TESTING Float operations
T{ 0.0 COS -> 1.0 }T   T{ PI COS -> -1.0 }T   T{ -PI COS -> -1.0 }T
T{ PI 2.0 F/ COS -> 0.0 }T   T{ -PI 2.0 F/ COS -> 0.0 }T

T{ 0.0 SIN -> 0.0 }T   T{ PI SIN -> 0.0 }T   T{ -PI SIN -> 0.0 }T
T{ PI 2.0 F/ SIN -> 1.0 }T   T{ -PI 2.0 F/ SIN -> -1.0 }T

T{ 4.0 SQRT -> 2.0 }T   T{ 4.0 EXP -> 54.598 }T
T{ 10.0 LOG -> 1.0 }T   T{ E LN -> 1.0 }T
)

\ -------------------------------------------------------------
TESTING Data-Stack manipulation
T{         DEPTH -> 0           }T
T{ 0       DEPTH -> 0 1         }T
T{ 0 1     DEPTH -> 0 1 2       }T
T{ 1       DUP   -> 1 1         }T
T{ 1 2     2DUP  -> 1 2 1 2     }T
T{ 1       DROP  ->             }T
T{ 1 2     DROP  -> 1           }T
T{ 1 2     2DROP ->             }T
T{ 0       ?DUP  -> 0           }T
T{  1      ?DUP  ->  1  1       }T
T{ -1      ?DUP  -> -1 -1       }T
T{ 1 2     SWAP  -> 2 1         }T
T{ 1 2     OVER  -> 1 2 1       }T
T{ 1 2 3   ROT   -> 2 3 1       }T
T{ 1 2     NIP   -> 2           }T
T{ 1 2 3   NIP   -> 1 3         }T
T{ 1 2     TUCK  -> 2 1 2       }T
T{ 1 2 3   TUCK  -> 1 3 2 3     }T
T{ 1 2 3 4 2OVER -> 1 2 3 4 1 2 }T
T{ 1 2 3 4 2SWAP -> 3 4 1 2     }T

\ -------------------------------------------------------------
TESTING Return-Stack manipulation.
\ Note: SimForth does not use the return stack because but an
\ auxiliary stack because in interpretation mode ANSI-Forths crash.
T{ : GR1 >R R> ;          -> }T
T{ : GR2 >R R@ 2R> DROP ; -> }T
T{ 123 GR1 -> 123 }T   T{ -123 GR1 -> -123 }T   T{ 65537 GR1 -> 65537 }T
T{ 123 GR2 -> 123 }T   T{ -123 GR2 -> -123 }T   T{ 65537 GR2 -> 65537 }T

( Deviation from ANSI-Forth: )
T{ 123 >R R> -> 123 }T            T{ -65537 >R R> -> -65537 }T
T{ 123 >R R@ 2R> DROP -> 123 }T   T{ -65537 >R R@ 2R> DROP -> -65537 }T

\ -------------------------------------------------------------
TESTING Check boolean operations
T{ FALSE -> 0 }T     T{ TRUE -> -1 }T
T{ 0 0 AND -> 0 }T   T{ 0 0 OR -> 0 }T   T{ 0 0 XOR -> 0 }T
T{ 0 1 AND -> 0 }T   T{ 0 1 OR -> 1 }T   T{ 0 1 XOR -> 1 }T
T{ 1 0 AND -> 0 }T   T{ 1 0 OR -> 1 }T   T{ 1 0 XOR -> 1 }T
T{ 1 1 AND -> 1 }T   T{ 1 1 OR -> 1 }T   T{ 1 1 XOR -> 0 }T

T{ FALSE FALSE AND -> FALSE }T   T{ FALSE FALSE OR -> FALSE }T   T{ FALSE FALSE XOR -> FALSE }T
T{ FALSE  TRUE AND -> FALSE }T   T{ FALSE  TRUE OR -> TRUE  }T   T{ FALSE  TRUE XOR -> TRUE  }T
T{ TRUE  FALSE AND -> FALSE }T   T{ TRUE  FALSE OR -> TRUE  }T   T{ TRUE  FALSE XOR -> TRUE  }T
T{ TRUE   TRUE AND -> TRUE  }T   T{ TRUE   TRUE OR -> TRUE  }T   T{ TRUE   TRUE XOR -> FALSE }T

T{ 1 -1 AND -> 1 }T  T{ -1 1 OR -> -1 }T   T{ 1 -1 XOR -> -2 }T

\ -------------------------------------------------------------
TESTING Check comparisons
T{ 0 0 == -> TRUE }T   T{  1 1 == -> TRUE }T   T{ -1 -1 == -> TRUE }T
T{ 1 0 == -> FALSE }T  T{ -1 0 == -> FALSE }T  T{  0  1 == -> FALSE }T
T{ 0 -1 == -> FALSE }T
T{  0  1 < -> TRUE  }T  T{ 1 2 < -> TRUE  }T   T{ -1  0 < -> TRUE  }T
T{ -1  1 < -> TRUE  }T  T{ 0 0 < -> FALSE }T   T{  1  1 < -> FALSE }T
T{  1  0 < -> FALSE }T  T{ 2 1 < -> FALSE }T   T{  0 -1 < -> FALSE }T
T{  1 -1 < -> FALSE }T
T{  0  1 <= -> TRUE  }T  T{ 1 2 <= -> TRUE  }T   T{ -1  0 <= -> TRUE  }T
T{ -1  1 <= -> TRUE  }T  T{ 0 0 <= -> TRUE  }T   T{  1  1 <= -> TRUE  }T
T{  1  0 <= -> FALSE }T  T{ 2 1 <= -> FALSE }T   T{  0 -1 <= -> FALSE }T
T{  1 -1 <= -> FALSE }T
T{  0  1 > -> FALSE }T  T{ 1 2 > -> FALSE }T  T{ -1  0 > -> FALSE }T
T{ -1  1 > -> FALSE }T  T{ 0 0 > -> FALSE }T  T{  1  1 > -> FALSE }T
T{  1  0 > -> TRUE }T   T{ 2 1 > -> TRUE  }T  T{  0 -1 > -> TRUE  }T
T{  1 -1 > -> TRUE }T
T{  0  1 >= -> FALSE }T  T{ 1 2 >= -> FALSE }T  T{ -1  0 >= -> FALSE }T
T{ -1  1 >= -> FALSE }T  T{ 0 0 >= -> TRUE  }T  T{  1  1 >= -> TRUE  }T
T{  1  0 >= -> TRUE }T   T{ 2 1 >= -> TRUE  }T  T{  0 -1 >= -> TRUE  }T
T{  1 -1 >= -> TRUE }T

\ -------------------------------------------------------------
\ Invert
T{ FALSE INVERT -> TRUE }T     T{ TRUE INVERT -> FALSE }T
T{ 1   INVERT -> -2 }T   T{ -2 INVERT -> 1 }T
T{ 2   INVERT -> -3 }T   T{ -3 INVERT -> 2 }T
T{ 1.0 INVERT -> -2 }T
T{ 1.1 INVERT -> -2 }T   \ 1.1 >INT gives 1 and 1 INVERT gives -2
T{ 1.5 INVERT -> -3 }T   \ 1.5 >INT gives 2 and 2 INVERT gives -3
T{ 1   INVERT -> -2 }T   T{  1.0 INVERT -> -2 }T
T{ -1.1 INVERT -> 0 }T   T{ -1.5 INVERT ->  1 }T
T{ 0 INVERT 1 AND -> 1 }T
T{ 1 INVERT 1 AND -> 0 }T

\ -------------------------------------------------------------
T{ 0 NEGATE -> 0 }T
T{ 1 NEGATE -> -1 }T
T{ -1 NEGATE -> 1 }T
T{ 2 NEGATE -> -2 }T
T{ -2 NEGATE -> 2 }T
T{ 0 ABS -> 0 }T
T{ 1 ABS -> 1 }T
T{ -1 ABS -> 1 }T

\ -------------------------------------------------------------
TESTING Variables
T{ VARIABLE V1 -> }T
T{  123 V1 ! -> }T   T{ V1 @ ->  123 }T
T{ -123 V1 ! -> }T   T{ V1 @ -> -123 }T
T{  65538 V1 ! -> }T   T{ V1 @ ->  65538 }T
T{ -65538 V1 ! -> }T   T{ V1 @ -> -65538 }T
(
T{  123.5 V1 ! -> }T   T{ V1 @ ->  123.5 }T
T{ -123.5 V1 ! -> }T   T{ V1 @ -> -123.5 }T
T{  65538.5 V1 ! -> }T   T{ V1 @ ->  65538.5 }T
T{ -65538.5 V1 ! -> }T   T{ V1 @ -> -65538.5 }T
)

\ -------------------------------------------------------------
TESTING High Level words

T{ : GT0 [ ] ; -> }T   T{ GT0 -> }T
T{ 1 ' DUP EXECUTE -> 1 1 }T

T{ : GT1 123 ; -> }T   T{ ' GT1 EXECUTE -> 123 }T

T{ : GT4 POSTPONE GT1 ; IMMEDIATE -> }T
T{ : GT5 GT4 ; -> }T
T{ GT5 -> 123 }T
T{ : GT6 345 ; IMMEDIATE -> }T
T{ : GT7 POSTPONE GT6 ; -> }T
T{ GT7 -> 345 }T

T{ : NOOP : POSTPONE ; ; -> }T
T{ NOOP NOP1 NOOP NOP2 -> }T   T{ NOP1 -> }T   T{ NOP2 -> }T

\ -------------------------------------------------------------
TESTING Conditionals
T{ : GI1 IF 123 THEN ; -> }T
T{ : GI2 IF 123 ELSE 234 THEN ; -> }T
T{ 0 GI1 -> }T
T{ 1 GI1 -> 123 }T
T{ -1 GI1 -> 123 }T
T{ 0 GI2 -> 234 }T
T{ 1 GI2 -> 123 }T
T{ -1 GI1 -> 123 }T

T{ : GI3 BEGIN DUP 5 < WHILE DUP 1+ REPEAT ; -> }T
T{ 0 GI3 -> 0 1 2 3 4 5 }T
T{ 4 GI3 -> 4 5 }T
T{ 5 GI3 -> 5 }T
T{ 6 GI3 -> 6 }T

T{ : GI4 BEGIN DUP 1+ DUP 5 > UNTIL ; -> }T
T{ 3 GI4 -> 3 4 5 6 }T
T{ 5 GI4 -> 5 6 }T
T{ 6 GI4 -> 6 7 }T

( FIXME KO
TRACES.ON
T{ : GI5 BEGIN DUP 2 >
         WHILE DUP 5 <  WHILE DUP 1+ REPEAT 123 ELSE 345 THEN ; WORDS -> }T

\EOF

T{ 1 GI5 -> 1 345 }T
T{ 2 GI5 -> 2 345 }T
T{ 3 GI5 -> 3 4 5 123 }T
T{ 4 GI5 -> 4 5 123 }T
T{ 5 GI5 -> 5 123 }T
)

T{ : GI6 ( N -- 0,1,..N ) DUP IF DUP >R 1- RECURSE R> THEN ; -> }T
T{ 0 GI6 -> 0 }T
T{ 1 GI6 -> 0 1 }T
T{ 2 GI6 -> 0 1 2 }T
T{ 3 GI6 -> 0 1 2 3 }T
T{ 4 GI6 -> 0 1 2 3 4 }T

\ -------------------------------------------------------------
TESTING Loops
T{ : GD1 DO I LOOP ; -> }T
T{ 4 1 GD1 -> 1 2 3 }T
T{ 2 -1 GD1 -> -1 0 1 }T

T{ : GD3 DO 1 0 DO J LOOP LOOP ; -> }T
T{ 4 1 GD3 -> 1 2 3 }T
T{ 2 -1 GD3 -> -1 0 1 }T

T{ : GD5 123 SWAP 0 DO I 4 > IF DROP 234 LEAVE THEN LOOP ; -> }T
T{ 1 GD5 -> 123 }T
T{ 5 GD5 -> 123 }T
T{ 6 GD5 -> 234 }T

\ -------------------------------------------------------------
TESTING INPUT: ACCEPT

\ -------------------------------------------------------------
TESTING DICTIONARY SEARCH RULES
T{ : GDX    123 ; : GDX    GDX 234 ; -> }T
T{ GDX -> 123 234 }T

T{ : DIV / ; : DIV DUP 0= IF 2DROP 42 RETURN ENDIF DIV ; -> }T
T{ 1 2 DIV -> 0 }T   T{ 1 0 DIV -> 42 }T
