\ forth_lib_expert_system.f

\ The entire reason I made this forth, or one of the big ones, was to have a forth of my own in C 
\ of my own that I can expand on what I learn and create from the tools and methods and practices etc explored in this book.
\ The book we're talking about is of course Designing and Programming Personal Expert Systems by Carl Townsend and
\ Dennis Feucht. In it, the system you see below is explored fundamentally and philosophically as a lisp like-language
\ built in this forth is used to create a prolog-like language for an expert system. That's all I know for now since I 
\ haven't gotten through the book fully yet. 

\ The the words in the forth engine used as the core of this whole system are eventually shaped to fit a forth close to the one
\ used in this book - OR - there may be a change or two in what the book does. We shall see. I'll be sure to document what I change. 
\ Or any other notable thoughts. Get ready for some rambling...

\ todo: write 'exists?' word to make sure I have all these?

\ The words required from this version of forth (FORTH-83)
\ for this particular system are the following:
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Nucleus Layer
\ !             2+              CMOVE>              MOD
\ #             2-              COUNT               NEGATE
\ */            2/              D+                  NOT
\ */MOD         <               D<                  OR
\ +             =               DEPTH               OVER
\ +!            >               DNEGATE             PICK
\ -             >R              DROP                R>
\ /             ?DUP            DEXECUTE            R@
\ /MOD          @               EXIT                ROLL
\ 0<            ABS             FILL                ROT
\ 0=            AND             I                   SWAP
\ 0>            C!              J                   U<
\ 1+            C@              MAX                 UM*
\ 1-            CMOVE           MIN                 UM/MOD
\                                                   XOR
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Device Layer
\ BLOCK         KEY
\ BUFFER        SAVE-BUFFERS
\ CR            SPACES
\ EXPECT        TYPE
\ FLUSH         UPDATE
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Interpreter Layer
\ @             <@              FIND                PAD
\ @>            >BODY           FORGET              QUIT
\ @S            >IN             FORTH               SIGN
\ @TIB          ABORT           FORTH-83            SPAN
\ ,             BASE            HERE                TIB
\ (             BLK             HOLD                U-
\ -TRAILING     CONVERT         LOAD                WORD
\ .             DECIMAL
\ .(            DEFINITIONS
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ Compiler Layer
\ +LOOP         ,               ."                  :
\ ;             DO              LOOP                VOCABULARY
\ ABORT"        DOES>           REPEAT              WHILE
\ ALLOT         ELSE            STATE               [
\ BEGIN         IF              THEN                [']
\ COMPILE       IMMEDIATE       UNTIL               [COMPILE]
\ CONSTANT      LEAVE           VARIABLE            ]
\ CREATE        LITERAL
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

\ Here, the book is not completely exhaustive in all the words used throughout.
\ And some words mentioned may not be used at all. I will keep track of anything 
\ that doesnt appear in the above table here (in no particular order):
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
\ 2DROP         NIP             -ROT
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

\ todo: add examples and definitions from the book?

vocabulary expert-system
definitions





in: forth

\ todo: engine == tty? or tty == engine?
\ todo: to use as little allocation as possible, each session keeps track of
\ how many nodes or lists and how much space it takes up. Next time we compile,
\ unless we malloc once during runtime, the values used to create these memory-
\ -regions or arenas change
