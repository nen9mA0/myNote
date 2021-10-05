; ========== half interval method ==========
(define (search f neg-point pos-point)
    (let ( (midpoint (average neg-point pos-point)) )
        (if (close-enough neg-point pos-point)
            midpoint
            (if (> (f midpoint) 0)
                (search f neg-point midpoint)
                (search f midpoint pos-point)
            )
        )
    )
)

(define (half-interval-method f a b)
    (let
        ( (a-value (f a))
          (b-value (f b)) )

        (cond
            ( (and (< a-value 0) (> b-value 0)) (search f a b) )
            ( (and (> a-value 0) (< b-value 0)) (search f b a) )
            (else (display "Value are not of opposite sign"))
        )
    )
)

; ========== fixed-point ==========
(define (fixed-point f first-guess)
    (define (try guess)
        (let
            ( (next (f guess)) )

            (if (close-enough guess next)
                next
                (try next)
            )
        )
    )
    (try first-guess)
)

(define (fixed-point-avr f first-guess)
    (define (try guess)
        (let
            ( (next (average (f guess) guess)) )

            (if (close-enough guess next)
                next
                (try next)
           )
        )
    )
    (try first-guess)
)

(define (average-damp f)
    (lambda (x) (average x (f x)))
)


; ========== newtons method ==========
(define dx 0.00001)

(define (deriv g)
    (lambda (x)
        (/ (- (g (+ x dx)) (g x)) dx)
    )
)

(define (newton-transform g)
    (lambda (x)
        ( - x (/ (g x) ((deriv g) x)) )
    )
)

(define (newtons-method g guess)
    (fixed-point (newton-transform g) guess)
)

; ===== =====

; use fixed-point-avr
(define (mysqrt x)
    (fixed-point-avr (lambda (y) (/ x y)) 1.0)
)

; use fixed-point
(define (mysqrt2 x)
    ( fixed-point ( average-damp (lambda (y) (/ x y)) ) 1.0 )
)

; use newtons-method
(define (mysqrt3 x)
    (newtons-method (lambda (y) (- (square y) x)) 1.0)
)
