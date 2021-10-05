(define (next x)
    (+ x 1)
)

(define (accumulate combiner null-value term a next b)
    (if (> a b)
        null-value
        (combiner (term a) (accumulate combiner null-value term (next a) next b))
    )
)

(define (accumulate2 combiner null-value term a next b)
    (define (accumulate2-iter combiner term a next b result)
        (if (> a b)
            result
            (accumulate2-iter combiner term (next a) next b (combiner (term a) result))
        )
    )

    (accumulate2-iter combiner term a next b null-value)
)