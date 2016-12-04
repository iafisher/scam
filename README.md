# TO DO
- Error handling
- UTF-8 support (?)

The Scam programming language is a dialect of Lisp based on Scheme.

## Syntax
The syntax of the language is described by the following EBNF grammar. Whitespace is only significant between symbols and numbers.

    EXPR    := "(" EXPR+ ")" | VALUE
    VALUE   := NUMBER | SYMBOL | LIST | STRING | QUOTE
    COMMENT := ";" <any ASCII char except newline> "\n"

    SYMBOL := <any SYMBOL_CHAR other than 0-9> SYMBOL_CHAR*
    LIST   := "[" EXPR* "]"
    QUOTE  := "{" EXPR* "}"
    STRING := "\"" <any ASCII char, with normal backslash escapes> "\""
    NUMBER := INTEGER | FLOAT

    SYMBOL_CHAR := /[A-Za-z0-9\-+?!%*/<>=_]/
    INTEGER := /-?[0-9]+/
    DECIMAL := /-?[0-9]+\.[0-9]+/

While negative integer literals could be matched by either the SYMBOL or INTEGER rule, they are always interpreted as integers.

## Semantics
### Named values
Scam has immutable *named values* rather than mutable *variables*--once a named value has been assigned, it cannot change over the lifetime of the program.

### Type system
All literals and named values belong to exactly one of the following types:

- Integer
- Decimal
- Boolean
- List
- String
- Quote
- Function
- Port

Scam is dynamically typed, so the types of named values need not (and in fact cannot) be declared at assignment. A list may contain elements of different types, and a function may return values of different types.

#### Integers, decimals and booleans
Integers and decimals are represented internally as C `long long`s and `double`s, respectively. This means that integers and decimals may overflow, and that decimal calculations will suffer from floating-point errors.

#### Lists and strings
The list is the primary general-purpose sequence type in Scam, and the string is a specialization of the list for sequences of ASCII characters.

#### Quotes
A quote value is a delayed-evaluation list. Symbols inside a quote will not be looked up, and S-expressions will not be evaluated.

#### Functions
Functions are first-class values in Scam. The notion of a function encapsulates the function body, the enclosing environment of the function, and the names of the parameters.

#### Ports
Port objects are used for input and output.

### Evaluation
A Scam expression is always evaluated in the context of an environment, which is a mapping from symbols to values. An environment may be enclosed by another environment; if a symbol has a mapping in both the enclosed and enclosing environment, then its value is from the mapping in the enclosed environment.

The following rules define the evaluation of Scam expressions.

- A literal (a number, string literal, boolean, list, or quote) evaluates to itself.
- A symbol evaluates to its value in the current environment, or to an error if it is not defined.
- An S-expression is evaluated as follows. The first element of the expression is evaluated, and if it is not a function, or if the number of remaining elements doesn't match the arity of the function, then an error is given. Otherwise, a new environment (enclosed by the function's environment) is created where the names of the parameters are bound to the arguments, and the body of the function is evaluated in this environment.

### Builtin functions and statements
The following special forms are available.

    (if <cond> <true-clause> <false-clause>)

An if-expression evaluates to the true clause if the condition is true, and to the false clause otherwise. The condition must evaluate to a boolean. At most one of the two clauses is ever evaluated.

    (define <identifier> <value>)
    (define (<function-name> <arg-list>) <body>)

The define statement creates a new binding in the current environment. It can only appear at the top level of a program, or at the beginning of a function body. Its return value is undefined.

    (lambda (<arg-list>) <body>)

The lambda expression creates an anonymous function. The function closes over its current environment, so that references to variables defined outside the function are still valid, no matter where the function is eventually invoked.

### Default environment
For a description of the functions defined in the default environment, see the `README-BUILTINS.md` file.
