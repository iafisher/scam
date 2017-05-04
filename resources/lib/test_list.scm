; empty?
>>> (empty? [])
true
>>> (empty? [1 2 3 4 5 6])
false
>>> (empty?)
ERROR
>>> (empty? 10)
ERROR
; len
>>> (len [])
0
>>> (len [1 2 3 4 5 6])
6
>>> (len)
ERROR
>>> (len 10)
ERROR
>>> (len [1 2] [3 4])
ERROR
; head
>>> (head [1 2 3 4 5 6])
1
>>> (head [])
ERROR
; tail
>>> (tail [1 2 3 4 5 6])
[2 3 4 5 6]
>>> (tail [])
[]
; last
>>> (last [1 2 3 4 5 6])
6
>>> (last [])
ERROR
; init
>>> (init [1 2 3 4 5 6])
[1 2 3 4 5]
>>> (init [])
[]
; prepend
>>> (prepend 0 [1 2 3 4 5 6])
[0 1 2 3 4 5 6]
>>> (prepend 0 [])
[0]
; append
>>> (append [1 2 3 4 5 6] 7)
[1 2 3 4 5 6 7]
>>> (append [] 0)
[0]
>>> (append "abcde" "f")
ERROR
; concat
>>> (concat [1 2 3] [4 5 6] [7 8 9])
[1 2 3 4 5 6 7 8 9]
>>> (concat ["a" true [-17.5 []]] [[42]])
["a" true [-17.500000 []] [42]]
>>> (concat [] [])
[]
>>> (concat "" "")
""
>>> (concat [] [1 2 3])
[1 2 3]
; get
>>> (get ["I" "met" "a" "traveller" "from" "an" "antique" "land"] 2)
"a"
; slice
>>> (slice [1 2 3 4 5 6 7 8 9] 3 6)
[4 5 6]
>>> (slice [] 0 0)
[]
>>> (slice "" 0 0)
""
>>> (slice [1 2 3] 2 7)
ERROR
>>> (slice [1 2 3] -1 2)
ERROR
>>> (slice [] 0 1)
ERROR
; take
>>> (take [3 1 4 1 5 9] 3)
[3 1 4]
>>> (take [1 2 3 4 5] 0)
[]
; drop
>>> (drop [3 1 4 1 5 9] 3)
[1 5 9]
>>> (drop [1 2 3 4 5] 0)
[1 2 3 4 5]
; insert
>>> (insert ["one" "small" "step" "for" "man"] 4 "a")
["one" "small" "step" "for" "a" "man"]
>>> (insert [1 2 3] 0 0)
[0 1 2 3]
>>> (insert [1 2 3] 3 4)
[1 2 3 4]
>>> (insert [] 0 "first")
["first"]
>>> (insert [1 2 3] -1 5)
ERROR
>>> (insert [1 2 3] 4 5)
ERROR
; find
>>> (find ["Germanic" "Slavic" "Romance"] "Germanic")
0
>>> (find ["Germanic" "Slavic" "Romance"] "Slavic")
1
>>> (find ["Germanic" "Slavic" "Romance"] "Romance")
2
>>> (find ["Germanic" "Slavic" "Romance"] "Indo-Iranian")
false
>>> (find ["Germanic" "Slavic" "Romance"] "romance")
false
>>> (find ["duplicate" "different" "duplicate"] "duplicate")
0
>>> (find [[1 1] [2 2] [3 3]] [1 1])
0
; rfind
>>> (rfind ["duplicate" "different" "duplicate"] "duplicate")
2
>>> (rfind ["Germanic" "Slavic" "Romance"] "Indo-Iranian")
false
>>> (rfind ["Germanic" "Slavic" "Romance"] "romance")
false
>>> (rfind [[1 1] [2 2] [3 3]] [1 1])
0
; sort
>>> (sort [5 4 3 2 1])
[1 2 3 4 5]
; map
>>> (map (lambda (x) (* x 2)) [1 2 3 4 5])
[2 4 6 8 10]
; filter
>>> (filter (lambda (x) (= (% x 2) 0)) [1 2 3 4 5 6 7 8 9 10])
[2 4 6 8 10]
; list, dict, str and repr
>>> (list 1 2 3 4 5)
[1 2 3 4 5]
