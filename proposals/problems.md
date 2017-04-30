## Bugs
### REPL does not allow comments
This is because I can't figure out the flex regular expression syntax to match EOF.


## Design flaws
### Standard library is unfinished

### Need more tests
There should be tests with restricted memory to make sure that the garbage collector works properly.

### Documentation is unfinished
Need docs for dictionary type and methods, and probably other things too (`split` for example).

### Dictionaries use inefficient implementation
Dictionaries are just arrays of keys and arrays of values. They aren't even sorted. A more efficient
hash table should be used instead.

### Garbage collector frees memory unnecessarily
When collection happens, memory is actually free, when it could just be marked free to save the
trouble of freeing and then reallocating it.

### Error messages suck
There needs to be a custom parser that can better identify syntax errors. Hopefully it could use
the flex tokenizer.

### Clearer names for the internal API
There should be a more direct relationship between the name of a type and the name of the function
that constructs it.

### Garbage collector uses global variables
Global variables means it's not thread-safe.

### The `eval_type` function is horrifying in every way
There's no good reason for `type` being a special function that the evaluator must handle: why can't
it just be a regular library function? Plus the way that types are stored has got to change.


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
