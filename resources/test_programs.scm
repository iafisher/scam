; basic palindrome program
>>> (define (palindrome s) (or (< (len s) 2) (and (= (head s) (last s)) (palindrome (slice s 1 (- (len s) 1))))))
>>> (palindrome "")
true
>>> (palindrome "a")
true
>>> (palindrome "ab")
false
>>> (palindrome "amanaplanacanalpanama")
true
>>> (palindrome "palindrome")
false
>>> (palindrome "noxinnixon")
true
; exponent algorithm
>>> (define (my-pow b n) (define (my-pow-helper ret n-so-far) (if (= n-so-far 0) ret (my-pow-helper (* b ret) (- n-so-far 1)))) (my-pow-helper 1 n))
>>> (my-pow 2 0)
1
>>> (my-pow 2 1)
2
>>> (my-pow 2 8)
256
