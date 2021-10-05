(define (sum term a next b)
    (define (iter a result)
        (if (> a b)
            result
            (iter (next a) (+ result (term a)))
        )
    )
    (iter a 0)
)

; (define (iter term a next b result)
;     (if (> a b)
;         result
;         (iter term (next a) next b (+ result (term a)))
;     )
; )

; (define (sum term a next b)
;     (iter term a next b 0)
; )


(define (cube x)
    (* x x x)
)

(define (next x)
    (+ x 1)
)