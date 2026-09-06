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
: fvariable floatsize make-variable ;
: v3variable 3 floatsize *  make-variable ;
: m3variable 3 3 * floatsize *  make-variable ;

variable consthere
variable consthere0

4096 cellsize * allot
dup consthere !
consthere0 !

variable datahere
variable datahere0
4096 cellsize * allot
dup datahere !
datahere0 !

1024 cellsize * allot
dup t0 !
tsp!

1024 floatsize * allot
dup f0 !
fsp!

\ : null-debugger-vector ;
\ ' null-debugger-vector debugger-vector !

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


: constalign consthere @ aligned consthere ! ;

: c, here @ c! here @ 1+ here ! ;
: const, consthere @ ! consthere @ cell+ consthere ! ;
: constc, consthere @ c! consthere @ 1+ consthere ! ;

: s" immediate
    state @ if            ( if compiling, emit a lit instruction with the starting pointer )
	consthere @          ( save string starting pos )
	begin
	    key     ( startpos key )
	    dup '"' <>  ( startpos key notadoublequote )
	while
		constc,
	repeat
	drop
	0 constc,  ( null-terminate! )
	' lit ,
	,              ( emit starting pos )
	constalign
    else
	consthere @
	begin
	    key
	    dup '"' <>
	while
		over c!
		1+
	repeat
	drop
	0 over c! drop
	consthere @
    then
;


: ." immediate
    state @ if
	[compile] s"
	' tell ,
    else
	begin
	    key
	    dup '"' = if
		drop exit
	    then
	    emit
	again
    then
;

: pick 1+ cellsize * dsp@ + @ ;

: make-const-str ( str -- conststr )
    dup consthere @
    strcpy drop
    consthere @ swap
    strlen 1+ consthere +!
    constalign
;

( sanakirjat )
variable current-vocab
variable latest-defined-vocab

0 current-vocab !
0 latest-defined-vocab !

: vocab-name ( vocabentry -- name ) cell+ @ ;
: vocab-next ( vocabentry -- nextvocabentry/0 ) 2 cells + @ ;
: vocab-latest ( vocab-entry -- latest ) @ ;
: set-vocab-name ( name vocabentry -- ) cell+ ! ;
: set-vocab-next ( nextentry vocabentry -- ) 2 cells + ! ;
: set-vocab-latest ( latest vocabentry -- ) ! ;
: vocab-useslist ( vocabentry -- useslist ) 3 cells + ;

\ : test-pick 10 20 30 2 pick . cr ;    \ expect 10 (three deep)
\ test-pick

\ : test-strlen s" hello" strlen . cr ;  \ expect 5
\ test-strlen

\ : test-strlen-compiled s" hello" strlen . cr ;
\ test-strlen-compiled

\ : test-raw-strlen
\     consthere @
\     dup 104 over c!         \ 'h'
\     dup 1+ 101 swap c!      \ 'e'
\     dup 2 + 108 swap c!     \ 'l'
\     dup 3 + 108 swap c!     \ 'l'
\     dup 4 + 111 swap c!     \ 'o'
\     dup 5 + 0 swap c!       \ null terminator
\     strlen . cr
\ ;
\ test-raw-strlen

: test-first-char
    s" hello" dup c@ .  cr    \ print the first byte's ASCII value
;
test-first-char