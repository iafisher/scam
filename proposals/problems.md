## Bugs
- REPL does not allow comments (This is because I can't figure out the flex regular expression syntax to match EOF.)
- Entering a blank line in the REPL gives an error.
- `concat` should be variadic.
- The unbound name error should include the name that triggered it.
- The test suite is not going to catch internal memory leaks, i.e. memory that the garbage collector holds a reference to even though it is dead. This memory won't show up on valgrind because the collector will deallocate it when the program finishes.


## Design flaws
- Standard library is unfinished
  - Need to implement `port-tell`, `port-seek`, `write`, and `join`
- There are no tests with restricted memory to make sure that the garbage collector works properly
- Dictionaries use an inefficient unsorted array implementation, when they could use a hash table instead.
- The garbage collector frees memory unnecessarily: it actually frees inaccessible objects rather than just marking them free and reusing the memory later.
- The syntax error messages suck because they use the builtin Bison messages, which are not very helpful. I should write a custom error parser that takes over when the Bison parser fails to give better messages.
- The garbage collector uses global variables. It might be helpful to have some kind of `program_state` object that has all the garbage collecting information as well as references to the interfaces of builtin types.
- The string conversion and printing functions contain a lot of redundancy and wasted memory allocation. There should be a single function that writes a scamval to a stream, and the string conversion function could determine the length of the string to be written, allocate a string of that length, and then pass a string stream to the `scamval_write` function.


## Features to add
- `let` expression
- `cond` expression
- the set as a builtin type
- error handling: raising and catching
- Unicode support
- module system
- object-oriented programming
- variadic functions
- convert AST to stack machine instructions???!!!
