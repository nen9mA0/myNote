(define (cont-frac n d k)
    (define (cont-frac-iter n d i)
        (if (< i k)
            (/ (n i) (+ (d i) (cont-frac-iter n d (+ i 1))) )
            (/ (n i) (d i))
        )
    )
    (cont-frac-iter n d 1)
)

(define (n i)
    1.0
)

(define (d i)
    (if (= (remainder (+ i 1) 3) 0)
        ( * 2 (/ (+ i 1) 3) )
        1
    )
)