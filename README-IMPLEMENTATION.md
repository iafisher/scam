The Scam programming language is implemented as a C program called `scam`. It is invoked in one of three ways: simply as `scam` to run the interactive REPL, as `scam <file name>` to run a file and exit, or as `scam -l <file name>` to run the interactive REPL with the definitions from the file loaded into the environment.

## Tokenizer
The tokenizer has the following interface to be used by the parser and for debugging. The current token can be accessed through the `tkn` member of the Tokenizer struct.

    // Initialize tokenizers from various sources
    Tokenizer* tokenizer_from_str(char*);
    Tokenizer* tokenizer_from_file(FILE*);

    // Move to the next token
    void tokenizer_advance(Tokenizer*);
    // Free all tokenizer resources, including the pointer itself
    void tokenizer_close(Tokenizer*);
    
    // Print all tokens and their type to stdout (useful for debugging)
    void print_all_tokens(Tokenizer*);

When tokenizer_advance is called, it first checks if its stream is open. If it is not, it sets the EOF token. Otherwise, it examines the first character in the stream; if it is a single character token, it yields that token. If it is the start of a string literal, it marks the stream and consumes characters until the end of the string is found. Otherwise, it marks the stream and continues consuming characters until either whitespace or a single character token is reached. Comments are not tokenized.

The type of a Token object is one of the following:

    TKN_RPAREN, TKN_LPAREN, TKN_RBRACKET, TKN_LBRACKET, TKN_RBRACE, TKN_LBRACE,
    TKN_INT, TKN_DEC, TKN_STR, TKN_EOF, TKN_UNKNOWN

### Stream
The tokenizer uses a stream internally so that character arrays and files can be handled with the same interface.

    // Initialize streams from various sources
    Stream* stream_from_str(char*);
    Stream* stream_from_file(FILE*);

    // Return 1 if the stream can still be read from
    int stream_good(Stream*);
    // Get the current character from the stream and advance to the next one
    char stream_getchar(Stream*);
    // Begin remembering the characters
    void stream_mark(Stream*);
    // Return all remembered characters and clear the memory
    char* stream_recall(Stream*);
    // Free all stream resources, including the pointer itself
    void stream_close(Stream*);

When `stream_mark` is called, the stream will save every character until the next call to `stream_recall`, which returns all the saved characters and clears the memory.

## Parser
Since the grammar of the language is LL(1), parsing is straightforward. The parser interface is given below. The two parse functions may return a `SCAM_ERR` value if parsing was unsuccessful.

    scamval* parse_line(char*);
    scamval* parse_file(FILE*);

The type of the return value can be anything except `SCAM_PORT` and `SCAM_FUNCTION`, since those types cannot be represented literally. In particular, the type of the return value is `SCAM_CODE` if the user entered an S-expression.

Internally, the parser is a recursive descent parser using the following functions.

    scamval* match_expr(Tokenizer*);
    scamval* match_expr_plus(Tokenizer*);
    scamval* match_expr_star(Tokenizer*);
    scamval* match_value_star(Tokenizer*);

## Evaluator
The evaluator can be accessed by any of these three functions, all three of which may modify the environment argument. The latter two functions invoke the parser and pass on its return value to `eval`.

    scamval* eval(scamval*, scamenv*);
    scamval* eval_line(char*, scamenv*);
    scamval* eval_file(FILE*, scamenv*);

If the `scamval` passed to `eval` is any of type other than `SCAM_SYM` or `SCAM_CODE`, then it is simply returned unchanged.

If it is of type `SCAM_SYM`, then its value in the given environment is returned, or an error is returned if it is not found.

If it is of type `SCAM_CODE`, then its first element is evaluated (if it has no element, an error is returned). If the first element does not evaluate to a function, then an error is returned. Otherwise, the rest of the elements are evaluated, and their values are bound to the names of the function's parameters (if too few or too many arguments are given, then an error is returned). Finally, the body of the function is evaluated in the extended environment.

### Representation of Scam values
Scam values implemented as a struct `scamval` with a `type` field and a `union` field that holds the possible values. The value of `type` indicates which field from the union should be accessed. `type` will be one of the following values:

    SCAM_INT, SCAM_DEC, SCAM_BOOL, SCAM_LIST, SCAM_STR, SCAM_QUOTE,
    SCAM_FUNCTION, SCAM_PORT, SCAM_BUILTIN, SCAM_CODE, SCAM_SYM, SCAM_ERR

The latter three types are only used internally and are never exposed to the user. The `SCAM_BUILTIN` type is always presented to the user identically to the `SCAM_FUNCTION` type, though they differ internally.
