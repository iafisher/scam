>>> (error)
ERROR
>>> (error "a")
ERROR
>>> (define s " b ")
>>> (error "a" s "c")
ERROR
>>> (begin (error) 42)
ERROR
>>> (if true 42 (error))
42
>>> (if false 42 (error))
ERROR
