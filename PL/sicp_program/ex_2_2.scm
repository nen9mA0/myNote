(define x-point car)
(define y-point cdr)
(define make-point cons)

(define (make-segment start end)
    (cons start end)
)

(define (start-segment segment)
    (car segment)
)

(define (end-segment segment)
    (cdr segment)
)

(define (midpoint-segment segment)
    (let
        (
            (start (start-segment segment))
            (end (end-segment segment))
        )

        ( make-point (average (x-point start) (x-point end)) (average (y-point start) (y-point end)) )
    )
)

(define (print-point p)
    (newline)
    (display "(")
    (display (x-point p))
    (display ",")
    (display (y-point p))
    (display ")")
)