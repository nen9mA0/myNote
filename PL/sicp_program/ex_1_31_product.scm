; recursion
(define (product term next a b)
    (if (> a b)
        1
        (* (term a) (product term next (next a) b))
    )
)

; iteration
(define (product2-iter term next a b result)
    (if (> a b)
        result
        (product2-iter term next (next a) b (* result (term a)))
    )
)

(define (product2 term next a b)
    (product2-iter term next a b 1)
)

(define (next x)
    (+ x 1)
)

(define (even n)
    (= (remainder n 2) 0)
)

(define (pi-next1 k)
    (if (even k)
        (+ k 2)
        (+ k 1)
    )
)

(define (pi-next2 k)
    (if (even k)
        (+ k 1)
        (+ k 2)
    )
)

(define (pi n)
    ( * 4 (/ (product2 pi-next1 next 1 n) (product2 pi-next2 next 1 n)) )
)

; (define (next n) (+ n 1))