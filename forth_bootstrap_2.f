
: \ immediate
    begin
	key
	'\n' <>
    while
    repeat
;

: ( immediate 
    1
    begin
	key
	dup '(' =
	if
	    drop 1+
	else
	    ')' =
	    if
		1-
	    then
	then
    dup 0=
    until
    drop
;