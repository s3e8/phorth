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


: find-vocabulary ( name -- vocabulary/0 )
    latest-defined-vocab @         ( name latestvocab )
    begin
	dup 0= if                  \ is the entry zero?
	    2drop 0 exit           \ return zero
	else
	    2dup vocab-name str<>   \ compare names
	then
    while
	    vocab-next
    repeat
    nip
;

: in: immediate
    word find-vocabulary
    ?dup if
	latest @ current-vocab @ set-vocab-latest   \ save latest to current vocabulary
	dup current-vocab !                         \ this is the new current vocabulary
	vocab-latest latest !                       \ get new latest from current vocabulary and save it to latest
    else
	." no such vocabulary" cr
    then
;

\ todo: prevent duplicate names later?
: vocabulary immediate
    word            ( vocabname )
    make-const-str  ( constvocabname )
    consthere @     ( constvocabname vocabulary )
    2dup set-vocab-name   ( constvocabname vocabulary )
    nip                   ( vocabulary )

    current-vocab @       ( vocabulary currentvocab )
    ?dup if
	latest @ swap set-vocab-latest
    then
    latest-defined-vocab @  ( vocabulary latestvocab )
    over set-vocab-next     ( vocabulary )  \ link them
    latest @                ( vocabulary currlatest )
    over set-vocab-latest   ( vocabulary )  \ save latest
    dup latest-defined-vocab !  \ make it the last defined vocab
    dup current-vocab !         \ it also becomes the current vocab like with in:
    vocab-useslist consthere !       \ advance consthere
;

: use immediate
    word find-vocabulary
    ?dup if
	const,
    else
	." no such vocabulary to use" cr
    then
;

: definitions immediate
    0 const,   \ terminate uses list
;

: vocabularies ( -- )
    latest-defined-vocab @
    begin
	dup
    while
	    dup vocab-name tell space
	    vocab-next
    repeat
    drop
;


\
\
\
\
\


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

\ \ safe version of find
\ ( vocabulary-aware new version of find )
\ : find ( wordname -- word )
\     dup find
\     ?dup if
\         nip exit
\     else
\         current-vocab @ 0= if
\             drop 0 exit
\         then
\         latest @
\         current-vocab @ vocab-useslist
\         begin
\             dup @
\         while
\ 		dup @ vocab-latest       ( wordname latest useslist usedlatest )
\ 		latest !                 ( wordname latest useslist )
\ 		2 pick                   ( wordname latest useslist wordname)
\ 		find                     ( wordname latest useslist word/0 )
\ 		?dup if
\ 		    nip over latest !
\ 		    2nip
\ 		    exit
\ 		else
\ 		    cell+
\ 		then
\ 	repeat
\ 	drop latest ! drop 0
\     then
\ ;

: ?hidden    @ f_hidden    and ;
: ?immediate @ f_immediate and ;
: ?builtin   @ f_builtin   and ;
: ?inline    @ f_inline    and ;

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

\ \ safer tick? 
\ : ' immediate
\     word find
\     dup 0= if
\         drop
\     else
\         dup ?builtin if
\             >cfa @
\         else
\             >cfa
\         then
\         state @ if
\             ' lit , ,
\         then
\     then
\ ;

: [compile] immediate
    word find
    dup @ f_builtin and
    if
	>cfa @ ,
    else
	' call , >cfa ,
    then
;


vocabulary forth
definitions

: hide word find hidden ;

hide latest-defined-vocab
hide vocab-next
hide vocab-latest
hide set-vocab-name
hide set-vocab-next
hide set-vocab-latest
hide vocab-useslist
hide find-vocabulary

variable firstbuiltin

: find-first-builtin ( -- )
    latest @
    begin
	dup ?builtin not
    while	    
	    cell+ @
    repeat
    firstbuiltin !
;

find-first-builtin

: find-bytecode ( bytecode -- dicthdr )
    firstbuiltin @     ( bytecode dictentry )
    begin
	2dup >cfa @ <>   ( bytecode dictentry issame? )
    while
	    cell+ @
    repeat
    nip
;

: ?hasarg ( dict-entry -- true/false )
    @ f_hasarg and ;

: ?iscall ( dict-entry -- true/false )
    >cfa @ ' call = ;

: copytohere ( addr -- addr+cellsize )
    dup @ , cell+
;

: perform-inline ( codetoinline -- )
    begin
	dup @ ' eow <>
    while
	    dup @
	    find-bytecode ?hasarg if
		copytohere
	    then
	    copytohere
    repeat
    here @ cell- here !
    drop
;

: interpret
    iword
    dup 0= if
	drop exit
    then
    dup find
    ?dup if
	nip
	dup ?immediate if
	    iexecute
	else
	    state @ if
		dup ?builtin if
		    >cfa @ ,
		else
		    dup ?inline if
			>cfa perform-inline
		    else
			' call , >cfa ,
		    then
		then
	    else
		iexecute
	    then
	then
    else
	dup number
	if
	    state @ if
		' lit ,
		,
		drop
	    else
		nip
	    then
	else
	    fnumber
	    if
		state @ if
		    ' flit ,
		    f,
		then
	    else
		." no such word" cr
	    then
	then
    then
;

\
\
\
\
\

: defer immediate
    create \ todo: make sure word and create are correct definitions
    latest @ @ f_deferred xor latest @ !
    ' jump ,
    0 ,
    ' exit ,
    ' eow ,
;

: is immediate
    word find
    ?dup if
	>cfa cell+ !
    else
	." no such word" cr
    then
;

: create ( wordname )
    dup find       ( wordname previousdef )
    ?dup if    \ if previous definition was found
	dup @ f_deferred and if  \ and it was deferred
	    over create    \ create new word  ( wordname dictentry )
	    >cfa cell+     ( wordname callptr )
	    latest @ >cfa  ( wordname callptr newwordimpl )
	    swap !
	    drop      \ wordname
	    exit
	then
	drop
    then    
    create
;

defer quit

: simple-quit
    begin
	?eof not
    while
	interpret
    repeat
    ." simple-quit done" cr
;

' simple-quit is quit