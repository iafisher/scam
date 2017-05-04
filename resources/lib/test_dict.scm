>>> (define dct {})
>>> dct
{}
>>> (define dct2 (bind dct 1 "one"))
>>> dct2
{1:"one"}
>>> dct
{}
>>> (get dct2 1)
"one"
>>> (dict [1 "one"] [2 "two"] [3 "three"])
{1:"one" 2:"two" 3:"three"}
; parse errors
>>> {1}
ERROR
>>> {1:}
ERROR
>>> {:}
ERROR
>>> {:"one"}
ERROR

