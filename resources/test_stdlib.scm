; addition
>>> (+ 1 1)
2
>>> (+ 1 2 3 4 5)
15
>>> (+ 1 2 3.0 4 5)
15.0
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
7.0
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
23.680000000000003
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
5.0
>>> (/ -72 2.5)
-28.8
>>> (/ 0 42)
0.0
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
; numeric equality
>>> (= 1 1)
true
>>> (= 1 1.0)
true
>>> (= 1.0 1)
true
>>> (= 1  0.999)
false
>>> (= 10 "10")
false
; string equality
>>> (= "abc" "abc")
true
>>> (= "" "")
true
>>> (= "hello" "hallo")
false
>>> (= "short" "a longer string")
false
; list equality
>>> (= [1 2 3] [1 2 3])
true
>>> (= [1 2 3] [1.0 2.0 3.0])
true
>>> (= [1 2 3] [1 2 3 4])
false
>>> (= [] [])
true
>>> (= ["a string" 10 true] ["a string" 10 true])
true
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
>>> (len "abc")
3
>>> (len "")
0
>>> (len)
ERROR
>>> (len 10)
ERROR
>>> (len [1 2] [3 4])
ERROR
; head
>>> (head [1 2 3 4 5 6])
1
>>> (head "abcdef")
"a"
>>> (head [])
ERROR
>>> (head "")
ERROR
; tail
>>> (tail [1 2 3 4 5 6])
[2 3 4 5 6]
>>> (tail "abcdef")
"bcdef"
>>> (tail [])
[]
>>> (tail "")
""
; last
>>> (last [1 2 3 4 5 6])
6
>>> (last "abcdef")
"f"
>>> (last [])
ERROR
>>> (last "")
ERROR
; init
>>> (init [1 2 3 4 5 6])
[1 2 3 4 5]
>>> (init "abcdef")
"abcde"
>>> (init [])
[]
>>> (init "")
""
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
["a" true [-17.5 []] [42]]
>>> (concat "ya pomnyu" " chudnoye mgnovenye")
"ya pomnyu chudnoye mgnovenye"
>>> (concat [] [])
[]
>>> (concat "" "")
""
>>> (concat [] [1 2 3])
[1 2 3]
; get
>>> (get ["I" "met" "a" "traveller" "from" "an" "antique" "land"] 2)
"a"
>>> (get "I met a traveller from an antique land" 6)
"a"
>>> (get "abc" 0)
"a"
>>> (get "abc" 2)
"c"
>>> (get "abc" 3)
ERROR
>>> (get "abc" -1)
ERROR
; slice
>>> (slice "no second Troy" 3 9)
"second"
>>> (slice [1 2 3 4 5 6 7 8 9] 3 6)
[4 5 6]
>>> (slice "abcdefg" 0 4)
"abcd"
>>> (slice "abcdefg" 3 7)
"defg"
>>> (slice [] 0 0)
[]
>>> (slice "" 0 0)
""
>>> (slice [1 2 3] 2 7)
ERROR
>>> (slice "abc" -1 2)
ERROR
>>> (slice "abc" 2 7)
ERROR
>>> (slice [1 2 3] -1 2)
ERROR
>>> (slice [] 0 1)
ERROR
>>> (slice "" 0 1)
ERROR
; take
>>> (take [3 1 4 1 5 9] 3)
[3 1 4]
>>> (take [1 2 3 4 5] 0)
[]
>>> (take "Gazing up into the darkness" 6)
"Gazing"
>>> (take "I saw myself as a creature" 0)
""
; drop
>>> (drop [3 1 4 1 5 9] 3)
[1 5 9]
>>> (drop [1 2 3 4 5] 0)
[1 2 3 4 5]
>>> (drop "Death be not proud" 13)
"proud"
>>> (drop "though some have called thee" 0)
"though some have called thee"
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
; upper
>>> (upper "abc")
"ABC"
>>> (upper "")
""
>>> (upper "string with whitespace, and punctuation!")
"STRING WITH WHITESPACE, AND PUNCTUATION!"
; lower
>>> (lower "ABC")
"abc"
>>> (lower "")
""
>>> (lower "STRING WITH WHITESPACE, AND PUNCTUATION!")
"string with whitespace, and punctuation!"
; trim
>>> (trim "  	abc")
"abc"
>>> (trim "abc		 ")
"abc"
>>> (trim "  abc 	 ")
"abc"
>>> (trim "no initial or final whitespace, but some internal")
"no initial or final whitespace, but some internal"
; split
>>> (split "quoth the Raven")
["quoth" "the" "Raven"]
>>> (split "	tabs	are	bad	")
["tabs" "are" "bad"]
>>> (split "word")
["word"]
>>> (split "")
[]
>>> (split " ")
[]
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
; sort
>>> (sort [5 4 3 2 1])
[1 2 3 4 5]
; map
>>> (map (lambda (x) (* x 2)) [1 2 3 4 5])
[2 4 6 8 10]
; filter
>>> (filter (lambda (x) (= (% x 2) 0)) [1 2 3 4 5 6 7 8 9 10])
[2 4 6 8 10]
; dictionary functions
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
; parse errors
>>> {1}
ERROR
>>> {1:}
ERROR
>>> {:}
ERROR
>>> {:"one"}
ERROR
; math library
>>> (ceil 10.7)
11
>>> (ceil -1.3)
-1

; list, dict, str and repr
>>> (list 1 2 3 4 5)
[1 2 3 4 5]
>>> (dict [1 "one"] [2 "two"] [3 "three"])
{1:"one" 2:"two" 3:"three"}
>>> (str {"North America":["United States"  "Canada"  "Mexico"]  "Asia":["China"  "Japan"]})
"{\"North America\":[\"United States\" \"Canada\" \"Mexico\"] \"Asia\":[\"China\" \"Japan\"]}"
>>> (str "abc")
"abc"
>>> (repr "abc")
"\"abc\""
