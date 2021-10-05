; =========== print version ==========
(define (fixed-point-print f first-guess)
    (define (try guess)
        (let
            ( (next (f guess)) )

            (newline)
            (display guess)
            (if (close-enough guess next)
                next
                (try next)
            )
        )
    )
    (try first-guess)
)

(define (fixed-point-avr-print f first-guess)
    (define (try guess)
        (let
            ( (next (average (f guess) guess)) )

            (newline)
            (display guess)
            (if (close-enough guess next)
                next
                (try next)
            )
        )
    )
    (try first-guess)
)
