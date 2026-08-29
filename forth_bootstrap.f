: if immediate
    ' 0branch ,
    here @ 
    0 ,
;

: then immediate
    dup
    here @ swap -
    swap !
;

: else immediate
    ' branch ,
    here @
    0 ,
    swap
    dup
    here @ swap -
    swap !
;

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