(define numer car)
(define denom cdr)

(define (make-rat n d)
    (let
        ( (g (GCD n d)) )

        (if (and (> n 0) (< d 0))
            ( cons -(abs (/ n g)) (abs (/ d g)) )
            ( cons (/ n g) (/ d g) )
        )
    )
)

(define (add-rat x y)
    ( make-rat (+ (* (numer x) (denom y)) (* (numer y) (denom x))) (* (denom x) (denom y)) )
)

(define (sub-rat x y)
    ( make-rat (- (* (numer x) (denom y)) (* (numer y) (denom x))) (* (denom x) (denom y)) )
)

(define (mul-rat x y)
    ( make-rat (* (numer x) (numer y)) (* (denom x) (denom y)))
)

(define (div-rat x y)
    ( make-rat (* (numer x) (denom y)) (* (denom x) (numer y)) )
)

(define (equal-rat x y)
    ( = (* (numer x) (denom y)) (* (denom x) (numer y)) )
)

(define (print-rat x)
    (newline)
    (display (numer x))
    (display "/")
    (display (denom x))
)