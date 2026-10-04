\ forth_lib_brainfuck.f

vocabulary brainfuck
definitions

\ todo: make helpers global?
: '+' 43 ;
: ',' 44 ;
: '-' 45 ;
: '.' 46 ;
: '<' 60 ;
: '>' 62 ;
: '[' 91 ;
: ']' 93 ;

: for-each-char ( str xt -- )
    swap >r >r                  \ R: str xt
    begin
        r> r> dup c@            ( xt str ch )
    ?dup while
        -rot 1+ >r dup >r       ( ch xt )   \ R: str+1 xt
        execute
    repeat
    2drop
;


variable tape
variable ptr
here tape ! 30000 allot

: bf-reset
    tape @ ptr !                \ pointer at tape cell 0
    tape @                      \ 
    begin                       \ walking address starts at cell 0
        dup tape @ 30000 + <    \ is walking address still below end-of-tape?
    while                       \ continue if so 
        0 over c! 1+            \ stores 0 at the walking address, leaves address on stack, then moves to next byte
    repeat                      \ check walking address again
    drop                        \ throw away walking address
;

: bf>  1 ptr +! ;               \ shorthand for : bf> ptr @ 1+ ptr ! ;
: bf< -1 ptr +! ;               \ shorthand for : bf> ptr @ 1- ptr ! ;
: bf+ ptr @ c@ 1+ ptr @ c! ;
: bf- ptr @ c@ 1- ptr @ c! ;
: bf. ptr @ c@ emit ;
: bf, key ptr @ c! ;
: bf@ ptr @ c@ ;

: bf-interpret-and-compile-char ( ch -- )       \ todo: rename to interpret-byte?
    case
        '>' of  postpone bf>  endof
        '<' of  postpone bf<  endof
        '+' of  postpone bf+  endof
        '-' of  postpone bf-  endof
        '.' of  postpone bf.  endof
        ',' of  postpone bf,  endof
        '[' of  postpone begin  postpone bf@  postpone while  endof
        ']' of  postpone repeat  endof
    endcase
;
: bf-begin-compilation ( "name" -- ) word (create) ' bf-reset call, ;

\ : bf: ( "name" -- )
\     bf-begin-compilation
\     begin
\         key dup ';' <> 
\     while 
\         bf-interpret-and-compile-char 
\     repeat 
\     drop
\     end,
\ ;
: bf: ( "name" -- )
    :
    postpone bf-reset
    begin key dup ';' <> while bf-interpret-and-compile-char repeat drop
    postpone ;
;

: bf-interpret-string ( str "name" -- ) bf-begin-compilation  ' bf-interpret-and-compile-char for-each-char  end, ;

\ todo: not needed for definitions right? 
\ hide tape  
\ hide ptr  
\ hide bf-reset  
\ hide bf>  
\ hide bf<  
\ hide bf+  
\ hide bf-
\ hide bf.  
\ hide bf,  
\ hide bf@
\ hide bf-interpret-and-compile-char  
\ hide bf-begin-compilation

in: forth