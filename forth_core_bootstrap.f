: make-inline
    latest @
    dup    @ f_inline xor
    swap !
;
: inline  immediate make-inline ;
: cell    inline cellsize   ;
: cells   inline cellsize * ;
: aligned cellsize 1- + cellsize 1- invert and ;
: align   here @ aligned here  ! ;
: allot   here @ swap    here +! align ;

: make-variable
    allot
    word (create) make-inline
    ' lit , , ' exit , ' eow ,
;
:   variable        cellsize    make-variable ;
:  fvariable       floatsize    make-variable ;
: v3variable 3     floatsize *  make-variable ;
: m3variable 3 3 * floatsize *  make-variable ;

\ : null-debugger-vector ;
\ ' null-debugger-vector debugger-vector !

: if immediate
    ' 0branch ,
    here @ 0 ,
;

: then immediate
    dup
    here @ swap -
    swap !
;

: else immediate
    ' branch ,
    here @ 0 ,
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

\ todo: use >xt instead ;
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


: spaces ( n -- )
    begin
	dup 0>
    while
	    space
	    1-
    repeat
    drop
;

: decimal immediate 10 base ! ;
: hex immediate 16 base ! ;

: f. s" %f" format tell ;

: u. ( u -- )
    base @ u/mod
    ?dup if
	recurse
    then

    dup 10 < if
	'0'
    else
	10 -
	'A'
    then
    +
    emit
;

: .ds ( -- )
    dsp@
    begin
	dup s0 @ u<
    while
	    dup @ u.
	    space
	    cell+
    repeat
    drop
;

: .ts ( -- )
    tsp@
    begin
	dup t0 @ u<
    while
	    dup @ u.
	    space
	    cell+
    repeat
    drop
;

: .fs ( -- )
    fsp@
    begin
	dup f0 @ u<
    while
	    dup f@ f. space
	    floatsize +
    repeat
    drop
;


: uwidth ( u -- width )
    base @ /
    ?dup if
	recurse 1+
    else
	1
    then
;

: u.r ( u width -- )
    swap
    dup
    uwidth
    rot
    swap -
    spaces
    u.
;

: .r
    swap
    dup 0< if
	negate
	1
	swap
	rot
	1-
    else
	0 swap rot
    then
    swap
    dup
    uwidth
    rot
    swap -
    spaces
    swap
    if
	'-' emit
    then
    u.
;

: . 0 .r space ; \ todo: why does this segfault? 
: u. u. space ;


\ vocabulary-aware new version of find
: find ( string -- word-header )
    dup find ?dup
    if
        nip exit
    else
        current-vocab @ 0= \ safe-guard for no-vocabulary case
        if
            drop 0 exit
        then
            latest @
            current-vocab @ vocab-useslist
        begin
            dup @
        while                  \ todo: ?
            dup @ vocab-latest ( wordname latest useslist usedlatest )
            latest !           ( wordname latest useslist )
            2 pick             ( wordname latest useslist wordname)
            find ?dup          ( wordname latest useslist word/0 )
            if
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

: ?hidden    @ f_hidden    and ;
: ?immediate @ f_immediate and ;
: ?builtin   @ f_builtin   and ;
: ?inline    @ f_inline    and ;

: ' immediate  ( better version of tick )
    word find dup 0=
    if  \ todo: improve error messages
	    ." Error: In TICK: No such word." cr 
        drop exit
    then
    dup ?builtin 
    if \ todo: use >xt
	    >cfa @
    else
	    >cfa
    then
    state @ 
    if
	    ' lit , ,
    then
;

: [compile] immediate
    word find dup @ f_builtin and
    if \ todo: this pattern is really common
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

\ todo: formatting
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
		." Error: In INTERPRET: No such word. (todo: print word)" cr
	    then
	then
    then
;

: defer immediate
    word (create) \ todo: make sure word and (create) are correct definitions
    latest @ @ f_deferred xor latest @ !
    ' jump ,
    0 ,
    ' exit ,
    ' eow ,
;

: is immediate
    word find ?dup 
    if
	    >cfa cell+ !
    else
	    ." Error: In IS: No such word." cr
    then
;

\ todo: is this the same as the reference?
: (create) ( wordname )
    dup find ?dup                ( wordname previousdef )
    if                           \ if previous definition was found
	    dup @ f_deferred and 
        if                       \ and it was deferred
            over (create)          \ (create) new word  ( wordname dictentry )
            >cfa cell+           ( wordname callptr )
            latest @ >cfa        ( wordname callptr newwordimpl )
            swap !
            drop                 \ wordname
            exit
	    then
	    drop
    then    
    (create)
;

defer quit

: simple-quit
    begin
	    ?eof not
    while
	    interpret
    repeat
;

' simple-quit is quit

( redefine to inline )
: cell  inline cellsize   ;
: cells inline cellsize * ;

\ new version of colon to support deferred words-aware (create)
: :
    word (create)
    latest @ hidden
    ]
;

hide copytohere
\ hide perform-inline

\ todo: why define here? ;
: cell+ inline cellsize + ;
: cell- inline cellsize - ;

: do immediate
    ' >r , ' >r ,
    [compile] begin 
;


: loop immediate
    ' r> , ' r> ,
    ' 1+ ,     \ add loop var
    ' 2dup , ' >r , ' >r ,
    ' = ,
    [compile] until
    ' rdrop , ' rdrop ,
;

: unloop immediate
    ' 2rdrop ,
;

: i inline ( -- loopvar ) rsp@ cell+ @ ;

\ : depth
\     s0 @ dsp@ -
\     cell-
\ ;

: fdepth
    f0 @ fsp@ -
;

: tdepth
    t0 @ tsp@ -
;

: ? @ . ;


: within
    -rot over
    <= if
	> if true else false then
    else
	2drop false then
;

: constant
    word (create)
    ' lit ,
    ,
    ' exit ,
;

: fconstant
    word (create)
    ' flit ,
    f,
    ' exit ,
;

: value
    word (create)
    ' lit ,
    ,
    ' exit ,
    ' eow ,
;

: to immediate
    word find
    >cfa
    cell+
    state @ if
	' lit ,
	,
	' ! ,
    else
	!
    then
;

: :noname
    0 (create)
    here @
    ] 
;

: ['] immediate [compile] ' ;
: compile, ( xt -- )  dup here0 here @ within if ' call , then , ;

\ ( create / does> )
\ a created word's code:  lit <body>  exit  0  eow  | body...
\ does> patches it to:     lit <body>  jump <does-code>  eow
: create ( "name" -- )
    word (create)
    ' lit ,  here @ 4 cells + ,      \ push body address (right after these 5 cells)
    ' exit ,  0 ,                     \ slot does> will patch into  jump <xt>
    ' eow ,
;
: (does>) ( does-code -- )
    latest @ >cfa 2 cells +          ( code slot )
    ['] jump over !  cell+ ! 
;
: does> immediate
    ' lit ,  here @ 4 cells + ,       \ address of the code after this does>
    ['] (does>) compile,              \ colon words are compiled as  call <xt>
    ' exit ,
;

: id. cell+ cell+ tell ;

: words
    latest @
    begin
	?dup
    while
	    dup ?hidden not if
		dup id.
		space
	    then
	    cell+ @
    repeat
    cr
;

\ include forth_peephole.f
\ include opt-word \ todo? 


variable compiling-lambda
0 compiling-lambda !

: :lambda immediate
    state @ if     \ if compiling
	' lit ,
	datahere @ ,
	here @     \ save old here ptr
	datahere @ here !    \ save new here for compilation
    else
	0 (create)
	here @
	1 compiling-lambda !
	]
    then
;

: ;; immediate
    state @ if
	' exit , ' eow ,
	compiling-lambda @ 0= if
	    here @ datahere !   \ advance consthere
	    here !    \ restore old here pointer
	else
	    [compile] [
	then
	0 compiling-lambda !
    then
;

: times immediate
    word find
    dup 0= if
	." times: no such word" cr
	drop exit
    then

    dup ?builtin if
	here @
	' >t ,
	over >cfa @ ,
	' t> ,
	' 1- ,
	' dup , ' 0>branch ,
	here @ - ,
	' drop ,
    else
	here @
	' >t ,
	over >cfa
	' call , ,
	' t> ,
	' 1- ,
	' dup , ' 0>branch ,
	here @ - ,
	' drop ,
    then
    drop
;

: exception-marker
    rdrop 0
;

: catch
    dsp@ cell+ >r
    ' exception-marker >r
    execute
;

defer breakpoint

: throw ( n -- )
    ?dup if
	rsp@                         ( n rsp )
	begin
	    dup r0 @ cell- u<
	while
		dup @
		' exception-marker = if
		    cell+
		    rsp!
		    dup dup dup
		    r>
		    cell-
		    swap over
		    !
		    dsp! exit
		then
		cell+
	repeat

	drop
	case
	    -1 of ." aborted" cr endof
	    
	    ." uncaught throw " dup . cr
	endcase
	breakpoint
    then
;

: lookup-word-from-ip
    latest @              ( codeaddr latest )
    begin
	?dup
    while
	    2dup swap     ( codeaddr latest latest codeaddr )
	    u< if
		nip
		exit
	    then
	    cell+ @
    repeat
    drop 0
;

: print-stack-trace
    rsp@
    begin
	dup r0 @ cell- <>
    while
	    dup @
	    case
		' exception-marker of ." catch ( dsp=" cell+ dup @ u. ." ) " cr endof

		dup
		lookup-word-from-ip
		
		id. cr
	    endcase
	    cell+
    repeat
    drop
    cr
;

: prompt-display-data
    current-vocab @ vocab-name
    fdepth floatsize /
    tdepth cell /
    depth 3 - \ todo: this uses the c-defined, cells-based depth.. revisit? 
;

\ todo: clean up prompt stuff \
defer prompt
: simple-prompt s" > ";
: format-prompt
    prompt-display-data s" [ds:%d ts:%d fs:%d %s]> "       format ;
: format-debugger-prompt
    prompt-display-data s" [ds:%d ts:%d fs:%d %s] DEBUG> " format ;
' simple-prompt is prompt

: bytes-used       here      @      here0   - ;
: const-bytes-used consthere @ consthere0 @ - ;
: data-bytes-used  datahere  @  datahere0 @ - ;

: usage
    ."         space used: " bytes-used       . cr
    ."   const space used: " const-bytes-used . cr
    ."    data space used: " data-bytes-used  . cr
;
: welcome
    ." Hello" cr
    usage
;

: r/o  s" r" ;  \ read only
: w/o  s" w" ;  \ write only
: r/w  s" r+" ; \ read/write \ todo: rename to rw? or is this ans
: a/o  s" a" ;  \ append only

: with-output  ( fp xt -- ) output-stream @ >r  swap output-stream ! catch r> output-stream ! throw ;                 

: >input-stack ( x -- )  input-stack-pointer @ !  cell input-stack-pointer +! ;
: input-stack> ( -- x )  cell negate input-stack-pointer +!  input-stack-pointer @ @ ;
: input-depth  ( -- n )  input-stack-pointer @ input-stack -  3 cells / ;

: save-input    ( -- )  input-stream @ >input-stack  input-buffer @ >input-stack  input-buffer-pos @ >input-stack ;
: restore-input ( -- )  input-stack> input-buffer-pos !  input-stack> input-buffer !  input-stack> input-stream ! ;

: next-input-buffer ( -- addr )  input-buffers  input-depth 1+  input-buffer-size * + ;

: interpret-file ( -- )
    begin refill while
	begin ?eol not while interpret repeat
    repeat
;

: included ( c-addr -- )
    input-depth 1+ input-stack-max-depth = if drop ." too many includes" cr exit then
    r/o open-file ?dup 0= if ." no such file" cr exit then
    next-input-buffer swap       ( buffer fp )
    save-input
    input-stream !
    dup input-buffer !  dup input-buffer-pos !  0 swap c!
    ['] interpret-file catch     ( 0 | throw-code )
    input-stream @ close-file
    restore-input
    throw                        \ re-throw after cleanup (0 throw does nothing)
;

: include ( "name" -- )  word included ;

: final-quit
    <stdin> input-stream !
    0 input-buffer @ c!  input-buffer @ input-buffer-pos !
    begin
	simple-prompt tell refill
    while
	    begin ?eol not while ' interpret catch drop repeat
	    cr
    repeat
    die
;

' final-quit is quit




: redirect-input-buffer ( fp "rest of line" -- )
    output-stream @ >r  output-stream !
    begin ?eol not while interpret repeat
    r> output-stream ! ;

: log<<    ( "rest of line" -- )  s" log.txt" a/o open-file dup redirect-input-buffer close-file ;
: stderr<< ( "rest of line" -- )  <stderr> redirect-input-buffer ;
: stdout<< ( "rest of line" -- )  <stdout> redirect-input-buffer ;

welcome
hide welcome


include forth_lib_tty.f

quit

\ todo: new ans (create)?
\ todo: add reset word to rebuild forth
\ todo: remove format prompt.. use in debug only or on command