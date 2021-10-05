(define (GCD a b)
    (if (= b 0)
        a
        (GCD b (remainder a b))
    )
)