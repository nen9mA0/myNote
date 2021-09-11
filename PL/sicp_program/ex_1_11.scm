(define (f1 n)
    (cond ( (< n 3) n)
          ( else (+ (f1 (- n 1)) (* 2 (f1 (- n 2))) (* 3 (f1 (- n 3))) ) )
    )
)

(define (f2-iter a b c count)
    (if (< count 3)
        a
        (f2-iter (+ a (* 2 b) (* 3 c)) a b (- count 1))
    )
)

(define (f2 n)
    (if (< n 3)
        n
        (f2-iter 2 1 0 n)
    )
)