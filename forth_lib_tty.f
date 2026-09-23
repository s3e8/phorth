: \r 13 ; \ carriage return - cr
: \n 10 ; \ line feed - lf
: crlf \r emit \n emit ;
: clear tty-clear-screen ;
: key-arrow-up    1000 ;
: key-arrow-down  1001 ;
: key-arrow-right 1002 ;
: key-arrow-left  1003 ;
: key-escape      27 ;
: quit-key? 113 = ; 
: tty-print-key
    dup 1000 = if ." UP"    crlf exit then
    dup 1001 = if ." DOWN"  crlf exit then
    dup 1002 = if ." RIGHT" crlf exit then
    dup 1003 = if ." LEFT"  crlf exit then
    dup . crlf  
;
: tty-read-key ( -- keycode )
    tty-read-byte
    dup key-escape = if
        drop
        tty-read-byte           ( second byte, expect '[' = 91 )
        91 = if
            tty-read-byte        ( third byte: A/B/C/D )
            case
                65 of key-arrow-up    endof
                66 of key-arrow-down  endof
                67 of key-arrow-right endof
                68 of key-arrow-left  endof
                key-escape           \ unrecognized sequence, fall back to plain ESC
            endcase
        else
            drop key-escape        \ ESC not followed by '['
        then
    then
;

: tty-handle-key ( keycode -- )
    tty-print-key
    drop
;

: tty 
    tty-enable-raw-mode
    begin
        tty-clear-screen
        tty-read-key
        dup quit-key? not
    while
        tty-handle-key
    repeat
    drop
    tty-disable-raw-mode
    tty-clear-screen
;