; addition
>>> (+ 1 1)
2
>>> (+ 1 2 3 4 5)
15
>>> (+ 1 2 3.0 4 5)
15.000000
>>> (+)
ERROR
>>> (+ 1)
ERROR
>>> (+ 1 [])
ERROR
; subtraction and negation
>>> (- 10)
-10
>>> (- 10 3)
7
>>> (- 10 3.0)
7.000000
>>> (- 10 8 2 3)
-3
>>> (-)
ERROR
>>> (- [])
ERROR
>>> (- 1 [])
ERROR
; multiplication
>>> (* 21 2)
42
>>> (* 3.2 7.4)
23.680000
>>> (* 1 2 3 4 5 6)
720
>>> (*)
ERROR
>>> (* 1)
ERROR
>>> (* 1 [])
ERROR
; real division
>>> (/ 10 2)
5.000000
>>> (/ -72 2.5)
-28.800000
>>> (/ 0 42)
0.000000
>>> (/)
ERROR
>>> (/ 10)
ERROR
>>> (/ 10 "abc")
ERROR
>>> (/ 10 0)
ERROR
>>> (/ 10 23 0)
ERROR
; floor division
>>> (// 10 3)
3
>>> (// 50 11 2)
2
>>> (// 0 42)
0
>>> (// 10 3.0)
ERROR
>>> (//)
ERROR
>>> (// 10)
ERROR
>>> (// 10 "abc")
ERROR
>>> (// 10 0)
ERROR
; remainder
>>> (% 73 2)
1
>>> (% 67 7)
4
>>> (% 0 10)
0
>>> (% 42 4.7)
ERROR
>>> (%)
ERROR
>>> (% 10)
ERROR
>>> (% 10 "abc")
ERROR
>>> (% 10 0)
ERROR
