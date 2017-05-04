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
