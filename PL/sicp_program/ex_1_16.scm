(define (expt1 b n)
    (if (= n 0)
        1
        (* b (expt1 b (- n 1)))
    )
)


(define (expt2-iter b product n)
    (if (= n 0)
        product
        (expt2-iter b (* b product) (- n 1))
    )
)


(define (expt2 b n)
    (expt2-iter b 1 n)
)


(define (even n)
    (= (remainder n 2) 0)
)


(define (fast-expt1 b n)
    (if (= n 0)
        1
        (if (even n)
            (square (fast-expt1 b (/ n 2)) )
            ( * b (fast-expt1 b (- n 1)) )
        )
    )
)


(define (fast-expt2-iter b n product)
    (cond
        ( (= n 0) product )
        ( (even n) (fast-expt2-iter (* b b) (/ n 2) product) )
        ( else (fast-expt2-iter b (- n 1) (* b product)) )
    )
)


(define (fast-expt2 b n)
    (fast-expt2-iter b n 1)
)
