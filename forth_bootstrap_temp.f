
: test-over 1 2 3 over . . . ;
1 2 3 test-over


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

: test-eq 5 5 = . 5 3 = . ;
test-eq

: test-case1
    depth .
    case
        1 of 111 . endof
    endcase
    depth .
;
1 test-case1
2 test-case1

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

: test-case2
    depth . cr
    case
        1 of 111 . endof
        2 of 222 . endof
    endcase
    depth . cr
;

1 test-case2
2 test-case2
3 test-case2

: test-case3
    depth . cr
    case
        1 of 111 . endof
        2 of 222 . endof
        3 of 333 . endof
    endcase
    depth . cr
;

1 test-case3
2 test-case3
3 test-case3
4 test-case3


: day-name
    case
        1 of 100 . endof
        2 of 200 . endof
        3 of 300 . endof
    endcase
    cr
;
1 day-name
2 day-name
3 day-name
4 day-name

: test-char char . ;
test-char A