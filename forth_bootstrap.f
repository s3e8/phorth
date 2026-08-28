: make-inline
    latest @ dup
    @ f_inline xor
    swap !
;

: inline immediate make-inline ;

: aligned cellsize 1- + cellsize 1- invert and ;
: align here @ aligned here ! ;

: allot here @ swap here +! align ;

: make-variable
    allot
    create make-inline
    ' lit ,
    ,
    ' exit ,
    ' eow ,
;

: variable cellsize make-variable ;

variable test1