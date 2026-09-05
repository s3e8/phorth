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
    swap
    dup
    here @ swap -
    swap !
;

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
    [compile] else
;
: endcase immediate
    ' drop ,
    begin ?dup while [compile] then repeat
;

: '\n' inline 10 ;
:  cr  inline 10 emit ;
: literal immediate ' lit , , ;
: char word c@ ;
: ':' inline [ char : ] literal ;
: ';' inline [ char ; ] literal ;
: '(' inline [ char ( ] literal ;
: ')' inline [ char ) ] literal ;
: '"' inline [ char " ] literal ;
: 'A' inline [ char A ] literal ;
: '0' inline [ char 0 ] literal ;
: '-' inline [ char - ] literal ;
: '.' inline [ char . ] literal ;