## Bugs
- REPL does not allow comments (This is because I can't figure out the flex regular expression syntax to match EOF.)


## Design flaws
- Standard library is unfinished
  - Need to implement `str`, `port-tell`, `port-seek`, and `write`
- There are no tests with restricted memory to make sure that the garbage collector works properly
- Dictionaries use an inefficient unsorted array implementation, when they could use a hash table instead.
- The garbage collector frees memory unnecessarily: it actually frees inaccessible objects rather than just marking them free and reusing the memory later.
- The syntax error messages suck because they use the builtin Bison messages, which are not very helpful. I should write a custom error parser that takes over when the Bison parser fails to give better messages.
- There should be a more direct relationship between the name of a type and the name of the function that constructs it.
- The garbage collector uses global variables. It might be helpful to have some kind of `program_state` object that has all the garbage collecting information as well as references to the interfaces of builtin types.


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

