; a comment
(define (my-range low high)
  (define (rec i seq-so-far)
    (if (= i high)
      seq-so-far
      (rec (+ i 1) (append seq-so-far i))))
  (rec low []))

(my-range 1 1000)
