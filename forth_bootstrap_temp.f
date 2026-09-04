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