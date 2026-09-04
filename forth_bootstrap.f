: make-inline
    latest @ dup
    @ f_inline xor
    swap !
;

: inline  immediate make-inline ;
: cell    inline cellsize   ;
: cells   inline cellsize * ;
: aligned cellsize 1- + cellsize 1- invert and ;
: align   here @ aligned here ! ;
: allot   here @ swap here +! align ;
: make-variable
    allot
    create make-inline
    ' lit ,
    ,
    ' exit ,
    ' eow ,
;

: variable cellsize make-variable ;

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
    bp
    swap
    dup
    here @ swap -
    swap !
;

: test-else
    0<>
    if
        1 .
    else
        2 .
    then
;
1 test-else    \ expect: 1
0 test-else    \ expect: 2


: recurse immediate
    ' call ,
    latest @
    >xt ,
;

: begin immediate
    here @
;

: until immediate
    ' 0branch ,
    here @ -
    ,
;

: again immediate
    ' branch ,
    here @ -
    ,
;

: while immediate
    ' 0branch ,
    here @
    0 ,
;

: repeat immediate
    ' branch ,
    swap
    here @ - ,
    dup
    here @ swap -
    swap !
;

: [compile] immediate
    word find
    dup @ f_builtin and
    if
	>cfa @ ,
    else
	' call , >cfa ,
    then
;

: unless immediate
    ' 0= ,
    [compile] if
;

: case immediate 0 ;
: of immediate
    ' over ,
    ' = ,
    [compile] if
    ' drop ,
;
: (of) immediate
    [compile] if
    ' drop ,
;
: endof immediate
    bp [compile] else
;
: endcase immediate
    ' drop ,
    begin ?dup bp while [compile] then repeat
;







\
\
\
\
\
\ test stuff
variable test1
: countdown
    begin
        dup .
        1-
        dup 0=
    until
    drop
;
5 countdown

\ todo: does recurse need fixing? 
: count-to-1
    dup .
    1-
    dup 0<>
    if recurse exit then
    drop
;
5 count-to-1

: begin-countdown
    begin
        dup .
        1-
        dup 0=
    until
    drop
;
5 begin-countdown

: test-qdup depth . ?dup depth . drop drop ;
5 test-qdup drop   \ expect depth 1, then 2
0 test-qdup drop   \ expect depth 1, then 1 (no dup on zero)

: test-unless
    0<>
    unless
        1 .
    then
;

3 test-unless    \ 3<>0 is true, unless(true)=false, nothing printed
0 test-unless    \ 0<>0 is false, unless(false)=true, prints 1

: test-unless 0<> unless 1 . then ;
3 test-unless
0 test-unless

: test-compile1 [compile] dup . drop ;
5 test-compile1   \ expect: 5 5   (dup then .)

: day-name
    depth
    case
        1 of 100 . endof
        2 of 200 . endof
        3 of 300 . endof
    endcase
    depth
;

1 day-name    \ expect: 100
2 day-name    \ expect: 200
3 day-name    \ expect: 300
4 day-name    \ expect: nothing printed, no crash, stack clean