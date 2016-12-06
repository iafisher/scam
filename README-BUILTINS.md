## Arithmetic and boolean functions
The following arithmetic and boolean functions are installed in the default environment. All of the functions except `not` except two or more arguments. `not` takes a single argument, and `-` acts as the negation operator when given only one argument.

    + - * / // % and or not = >= <= > < !=

The `/` operator performs floating point division regardless of the types of its operands, which `//` operator only performs floor division on integers. `//` and `%` are the only two arithmetic operators with type restrictions: all others work on operands of any numerical type.

The comparison operators are defined for integers, decimals, and strings. The `=` operator is additionally usefully defined for lists and booleans; it returns `false` for all other types, and with mixed types (unless both are numeric).

## Sequence functions
### Defined for all sequence types (quotes, strings and lists)

    (head seq)

Return the first element of the sequence, or raise an error if the sequence is empty.

    (tail seq)

Return every element in the sequence after the first element. If the sequence is empty, return the empty sequence.

    (last seq)

Return the last element in the sequence, or raise an error if the sequence is empty.

    (init seq)

Return every element in the sequence before the last element. If the sequence is empty, return the empty sequence.

    (len seq)

Return the length of the sequence.

    (empty? seq)

Return `true` if the sequence contains no elements, `false` otherwise.

### Defined for strings and lists

    (prepend val seq)

Prepend the value to the sequence (note the reversed order of the sequence and value).

    (append seq val)

Append the value to the sequence.

    (concat seq1 seq2)

Concatenate two sequences.

    (get seq i)

Return the `i`th element of the sequence, or raise an error if `i` is out of bounds.

    (slice seq start end)

Return the subsequence whose first element is at index `start` and whose last element is at index `end - 1`. An error is raised if `start` or `end` are out of bounds.

    (find seq val)

Return the index where the value first appears in the sequence, or `false` if the value is not in the sequence.

    (rfind seq val)

Return the index where the value last appears in the sequence, or `false` if the value is not in the sequence.

## String functions

    (upper s)

Return the string where all of the alphabetic characters have been changed to uppercase.

    (lower s)

Return the string where all of the alphabetic characters have been changed to lowercase.

    (trim s)

Return the string where all whitespace has been removed from the beginning and end.

## I/O functions

    (getn port n)

Read `n` characters from the given port.

    (getline port)

Read a line from the given port. The resulting string will include a newline at the end.

    (write port obj)

Write the string representation of the object to the given port.

    (inputn n)

Read `n` characters from stdin.

    (input)

Read a line from stdin. The resulting string will include a newline at the end.

    (print obj)

Write the string representation of the object to stdout.

    (open name mode)

Open a new port with the given name (usually a file path) in the given mode. For details on valid modes, see the documentation for the C function `fopen`.

    (port-good? port)

Return True if the port is available for reading or writing.

    (port-tell port)

Return the current position of the port.

    (port-seek port i)

Set the current position of the port.

    (close port)

Close the port. After a port has been closed, any read or write operation will produce an error.

## Other functions

    (eval qu)

Evaluate a quote in the current environment.
