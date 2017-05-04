; IO functions (these won't work depending on what directory you are in)
>>> (define fp (open "resources/foo.txt" "r"))
>>> (port-good? fp)
true
>>> (readline fp)
"Lorem ipsum\n"
>>> (port-good? fp)
true
>>> (readline fp)
ERROR
>>> (port-good? fp)
false
>>> (close fp)
