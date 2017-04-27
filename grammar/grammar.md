## Grammar rules

    program -> block
    block -> statement_or_expression+
    statement_or_expression -> statement | expression
    statement -> define_function | define_variable
    define_function -> '(' DEFINE symbol_list block ')'
    define_variable -> '(' DEFINE SYMBOL expression ')'
    expression -> value | s_expression
    s_expression -> '(' expression+ ')'
    value -> STRING | INTEGER | DECIMAL

## Lexical rules

    DEFINE: define
    SYMBOL: [A-Za-z&^%.+/*_-][A-Za-z&^%.+/*_0-9-]*
    STRING: \"(\\.|[^\\"])\"
    INTEGER: -?[0-9]+
    DECIMAL: -?[0-9]+\.[0-9]+
