>>> (len "abc")
3
>>> (len "")
0
>>> (head "abcdef")
"a"
>>> (head "")
ERROR
>>> (tail "abcdef")
"bcdef"
>>> (tail "")
""
>>> (last "abcdef")
"f"
>>> (last "")
ERROR
>>> (init "abcdef")
"abcde"
>>> (init "")
""
>>> (concat "ya pomnyu" " chudnoye mgnovenye")
"ya pomnyu chudnoye mgnovenye"
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
>>> (slice "no second Troy" 3 9)
"second"
>>> (slice "abcdefg" 0 4)
"abcd"
>>> (slice "abcdefg" 3 7)
"defg"
>>> (slice "abc" -1 2)
ERROR
>>> (slice "abc" 2 7)
ERROR
>>> (slice "" 0 1)
ERROR
>>> (take "Gazing up into the darkness" 6)
"Gazing"
>>> (take "I saw myself as a creature" 0)
""
>>> (drop "Death be not proud" 13)
"proud"
>>> (drop "though some have called thee" 0)
"though some have called thee"
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
>>> (str {"North America":["United States"  "Canada"  "Mexico"]  "Asia":["China"  "Japan"]})
"{\"North America\":[\"United States\" \"Canada\" \"Mexico\"] \"Asia\":[\"China\" \"Japan\"]}"
>>> (str "abc")
"abc"
>>> (repr "abc")
"\"abc\""
