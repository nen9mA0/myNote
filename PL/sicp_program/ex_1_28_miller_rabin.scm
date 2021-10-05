(define (expmod base exp m)
    (cond
        ( (= exp 0) 1 )
        ( (nontrivial base m) 0 )
        ( (even exp) (remainder (square (expmod base (/ exp 2) m)) m) )
        (else (remainder (* base (expmod base (- exp 1) m)) m))
    )
)

(define (nontrivial a n)
    (cond
        ( (= a 1)  false )
        ( (= a (- n 1))  false )
        ( (= (remainder (square a) n) 1) true )
        ( else false)
    )
)

(define (fermat-test n)
    (define (try-it a)
        (= (expmod a (- n 1) n) 1)
    )
    (try-it (+ 1 (random (- n 1))))
)

(define (miller-rabin n times)
    (cond
        ( (= times 0) true )
        ( (fermat-test n) (miller-rabin n (- times 1)))
        (else false)
    )
)

(define (even n)
    (= (remainder n 2) 0)
)