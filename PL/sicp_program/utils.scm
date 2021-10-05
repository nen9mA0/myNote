(define tolerance 0.00001)

(define (average a b)
    (/ (+ a b) 2)
)

(define (close-enough a b)
    (< (abs (- a b)) tolerance)
)