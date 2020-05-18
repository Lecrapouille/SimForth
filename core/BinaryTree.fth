\ https://rosettacode.org/wiki/Tree_traversal#Forth
\ binary tree (dictionary)

: NODE ( l r data -- node ) HERE >R TOKEN, TOKEN, TOKEN, R> ;
: LEAF ( data -- node ) 0 0 ROT NODE ;

: >DATA  ( node -- ) TOKEN@ ;
: >RIGHT ( node -- ) TOKEN + TOKEN@ ;
: >LEFT  ( node -- ) TOKEN + TOKEN + TOKEN@ ;

: PREORDER ( xt tree -- )
  DUP 0= IF 2DROP RETURN THEN
  2DUP >DATA SWAP EXECUTE
  2DUP >LEFT RECURSE
       >RIGHT RECURSE
;

: INORDER ( xt tree -- )
  DUP 0= IF 2DROP RETURN THEN
  2DUP >LEFT RECURSE
  2DUP >DATA SWAP EXECUTE
       >RIGHT RECURSE
;

: POSTORDER ( xt tree -- )
  DUP 0= IF 2DROP RETURN THEN
  2DUP >LEFT RECURSE
  2DUP >RIGHT RECURSE
       >DATA SWAP EXECUTE
;

: MAX-DEPTH ( tree -- n )
  DUP 0= IF RETURN THEN
  DUP  >LEFT RECURSE
  SWAP >RIGHT RECURSE MAX 1+
;

( TODO
DEFER DEPTHACTION
: DEPTHORDER ( depth tree -- )
  DUP 0= IF 2DROP RETURN THEN
  OVER 0=
  IF   >DATA DEPTHACTION DROP
  ELSE OVER 1- OVER >LEFT  RECURSE
       SWAP 1- SWAP >RIGHT RECURSE
  THEN
;

: LEVELORDER ( xt tree -- )
  SWAP IS DEPTHACTION
  DUP MAX-DEPTH 0 ?DO
    I OVER DEPTHORDER
  LOOP DROP
;
)

\ Example:
\
\ 7 LEAF 0      4 NODE
\               5 LEAF 2 NODE
\ 8 LEAF 9 LEAF 6 NODE
\               0      3 NODE 1 NODE VALUE TREE
\
\ ' . TREE PREORDER    \ 1 2 4 7 5 3 6 8 9
\ ' . TREE INORDER     \ 7 4 2 5 1 8 6 9 3
\ ' . TREE POSTORDER   \ 7 4 5 2 8 9 6 3 1
\ TREE MAX-DEPTH .     \ 4
\ ' . TREE LEVELORDER  \ 1 2 3 4 5 6 7 8 9
