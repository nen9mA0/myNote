(define (halve n)
    (/ n 2)
)

(define (double n)
    (* n 2)
)

(define (even n)
    (= (remainder n 2) 0)
)


(define (fast-mul-iter a b n)
    (cond
        ( (= b 0) n )
        ( (even b) (fast-mul-iter (double a) (halve b) n) )
        ( else (fast-mul-iter a (- b 1) (+ n a)) )
    )
)

(define (fast-mul a b)
    (fast-mul-iter a b 0)
)