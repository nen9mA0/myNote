(define (sum func a b)
    (if (> a b)
        0
        (+ (func a) (sum func (+ a 1) b))
    )
)

(define (func x)
    (square x)
)