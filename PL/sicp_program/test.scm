(define (inc x)
    (+ x 1)
)

(define (test)
    (define x (inc 1))
    (define y (inc 2))
    (let
        (
            (z (inc y))
        )

        z
    )
)