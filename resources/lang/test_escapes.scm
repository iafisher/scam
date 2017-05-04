>>> (define s "\a\b\f\n\r\t\v\\\'\"\?")
>>> (len s)
11
>>> s
"\a\b\f\n\r\t\v\\'\"?"
>>> (len "\\\"")
2
; test some invalid escapes
>>> "\c"
ERROR
>>> "\z"
ERROR
