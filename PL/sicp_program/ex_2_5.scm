(define (cons a b)
    (* (expt 2 a) (expt 3 b))
)


(define (car x)
    (define (car-iter tmp a)
        (if (= (remainder tmp 2) 0)
            (car-iter (/ tmp 2) (+ a 1))
            a
        )
    )

    (car-iter x 0)
)

(define (cdr x)
    (define (cdr-iter tmp b)
        (if (= (remainder tmp 3) 0)
            (cdr-iter (/ tmp 3) (+ b 1))
            b
        )
    )

    (cdr-iter x 0)
)
