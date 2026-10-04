# phorth
WIP forth

### todo:
- [ ] count and type words
- [ ] debugger.. push graph nodes whether int or ptr etc
- [ ] unhide bf internals, just use vocab
- [ ] (create) to create-header
    - [ ] create() in dictionary also becomes create_header()
- [ ] word dependency graph (DEFCODE macro? )
- [ ] move prompt out of get_next_line
- [ ] dependency graph
    - [ ] generic graph
- [ ] : help: word print-usage ;
    - [ ] hash table
- [ ] rename <stdin> to stdin etc
- [ ] ans allot? 
- [x] include stack
- [ ] revert 'create' to original functionality
- [ ] print-ds
- [ ] debug interpreter
- [ ] general clean-up
- [ ] clean up macros and ops file
    - [ ] change 'ops' to 'builtin'
- [ ] platform_xxx + sys_tty
    - [ ] platform has file-local globals for each platform.. for now
    - [ ] just use platform naming convention.. maybe
- [ ] raylib bindings
    - [ ] conditional build cfg macro in forth.h
- [ ] do todos
- [ ] expert system
- [ ] builtin string space and scratch buffer
- [ ] rename core files to forth_core? like forth_lib
- [ ] step through debugger?
- [ ] look-up-word-from-ip .. make in C too
- [ ] clean up builtin macros
