(define (halve n)
    (/ n 2)
)

(define (double n)
    (* n 2)
)

(define (even n)
    (= (remainder n 2) 0)
)

(define (fast-mul a b)
    (cond
        ( (= a 0) 0 )
        ( (even a) (double (fast-mul (halve a) b)) )
        ( else ( + b (fast-mul (- a 1) b) ) )
    )
)