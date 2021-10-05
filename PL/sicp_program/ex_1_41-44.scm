(define (inc x)
    (+ x 1)
)

(define (double func)
    (lambda (x) (func (func x)))
)

(define (compose f g)
    (lambda (x) (f (g x)))
)

(define (repeated func n)
    (if (> n 1)
        ( compose func (repeated func (- n 1)) )
        (lambda (x) (func x))
    )
)

(define dx 0.01)

(define (smooth f)
    ( lambda (x) (/ (+ (f x) (f (+ x dx)) (f (- x dx))) 3) )
)
