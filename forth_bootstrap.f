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


: cell+ inline cellsize + ;
: cell- inline cellsize - ;

: str= inline strcmp 0= ;
: str< inline strcmp 0< ;
: str> inline strcmp 0> ;
: str<> inline strcmp ;

: bl inline  32 ;
: space inline bl emit ;
: negate inline 0 swap - ;
: true inline 1 ;
: false inline 0 ;
: not inline 0= ;

\ : min 2dup < if drop else nip then ;
\ : max 2dup > if drop else nip then ;
\ : fmin f2dup f< if fdrop else fnip then ;
\ : fmax f2dup f> if fdrop else fnip then ;


( vocabulary-aware new version of find )
: find ( wordname -- word )
    dup find            ( wordname dictentry ) \ try to find from current latest first
    ?dup if
	nip exit
    else
	latest @                         ( wordname latest )
	current-vocab @ vocab-useslist   ( wordname latest useslist )
	begin
	    dup @                        ( wordname latest useslist vocabentry/0 )
	while
		dup @ vocab-latest       ( wordname latest useslist usedlatest )
		latest !                 ( wordname latest useslist )
		2 pick                   ( wordname latest useslist wordname)
		find                     ( wordname latest useslist word/0 )
		?dup if
		    nip over latest !
		    2nip
		    exit
		else
		    cell+
		then
	repeat
	drop latest ! drop 0
    then
;

: ?hidden @ f_hidden and ;
: ?immediate @ f_immediate and ;
: ?builtin @ f_builtin and ;
: ?inline @ f_inline and ;

: ' immediate  ( better version of tick )
    word find
    dup 0= if
	." no such word" cr drop
	exit
    then
    dup ?builtin if
	>cfa @
    else
	>cfa
    then
    state @ if
	' lit , ,
    then
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