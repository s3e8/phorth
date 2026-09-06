: expect ( actual expected -- )
    2dup = if
        2drop ." PASS" cr
    else
        ." FAIL: got " swap . ." expected " . cr
    then
;

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

: test-quotes ':' . ';' . '(' . ')' . '"' . 'A' . '0' . '-' . '.' . ;
test-quotes    \ expect: 58 59 40 41 34 65 48 45 46

\ todo: test comments in immediate definitions

: test-1+ 5 1+ . cr ;                          \ expect 6
: test-lt 3 5 < . cr ;                         \ expect 1
: test-gt 5 3 > . cr ;                         \ expect 1
: test-0lt -1 0< . cr ;                        \ expect 1
: test-0gt 1 0> . cr ;                         \ expect 1
test-1+ test-lt test-gt test-0lt test-0gt

consthere @ .  cr
5 constc,
consthere @ .  cr
10 const,
consthere @ .  cr
consthere0 @ .  cr

variable v1
42 v1 !
v1 @ . cr

fvariable fv1

: test-misc
    5 negate . cr
    true . cr
    false . cr
    0 not . cr
    1 not . cr
    space 65 emit cr
;
test-misc

." hello world" cr
: greet ." compiled hello" cr ;
greet

: test-s"
    s" test string" tell cr
;
test-s"

: test-depth depth . cr s" abc" tell cr depth . cr ;
test-depth

: test-two-strings
    s" first" tell cr
    s" second" tell cr
;
test-two-strings


: test-pick 10 20 30 2 pick . cr ;    \ expect 10 (three deep)
test-pick

: test-strlen s" hello" strlen . cr ;  \ expect 5
test-strlen

: test-strlen-compiled s" hello" strlen . cr ;
test-strlen-compiled

: test-raw-strlen
    consthere @
    dup 104 over c!         \ 'h'
    dup 1+ 101 swap c!      \ 'e'
    dup 2 + 108 swap c!     \ 'l'
    dup 3 + 108 swap c!     \ 'l'
    dup 4 + 111 swap c!     \ 'o'
    dup 5 + 0 swap c!       \ null terminator
    strlen . cr
;
test-raw-strlen

: test-first-char
    s" hello" dup c@ .  cr    \ print the first byte's ASCII value
;
test-first-char

: test-2dup 1 2 2dup . . . . cr ;      \ expect 2 1 2 1
test-2dup

: test-2drop 1 2 3 4 2drop . . cr ;    \ expect 2 1
test-2drop

: test-nip 1 2 nip . cr ;              \ expect 2
test-nip

: test-strneq s" abc" s" abc" str<> . cr ;   \ expect 0 (equal)
: test-strneq2 s" abc" s" xyz" str<> . cr ;  \ expect nonzero
test-strneq test-strneq2

: test-loop-exit
    begin
        dup 3 =
        if
            drop 999 . cr
            exit
        then
        dup .
        1+
    again
;
0 test-loop-exit    \ expect: 0 1 2 999, then stops cleanly

s" nonexistent" find-vocabulary . cr   \ expect 0, since latest-defined-vocab is still 0

: test-nip2 1 2 nip 2 expect ;
test-nip2

: test-const-str
    s" abc"
    make-const-str
    dup tell cr
;
test-const-str

: test-newtick-ok ' dup drop ." ok" cr ;
test-newtick-ok

: test-newtick-fail ' totallynotarealword drop ." shouldn't reach here" cr ;
test-newtick-fail


\ \ : test-newtick-ok ' dup drop ." ok" cr ;
\ \ test-newtick-ok

\ \ : test-newtick-fail ' totallynotarealword drop ." shouldn't reach here" cr ;
\ \ test-newtick-fail

\ : test-newtick-fail2 ' totallynotarealword drop cr ;
\ test-newtick-fail2

: test-tick-fail-clean
    depth .          \ record depth before
    ' totallynotarealword
    depth .          \ record depth after — should match, since ' produced nothing to drop
    cr
;
test-tick-fail-clean



: test-find-fail-clean
    depth .
    s" totallynotarealword" find
    .                \ print whatever find actually returned (should be 0)
    depth .
    cr
;
test-find-fail-clean