; integral
(define (sum func next a b)
    (if (> a b)
        0
        (+ (func a) (sum func next (next a) b))
    )
)

(define (integral f a b dx)
    (define (add-dx x) (+ x dx))
    (* (sum f add-dx (+ a (/ dx 2.0)) b) dx)   ; add-dx是这个积分过程使用的next函数
)

(define (cube x)
    (* x x x)
)

; simpson integral

(define (simpson-integral f a b n)
    (define (h-func a b)
        (/ (- b a) n)
    )

    (define (y func a k h)
        (func (+ a (* k h)))
    )

    (define (even x)
        (= (remainder x 2) 0)
    )

    (define (next k)
        (cond
            ((= k 0) 1)
            ((= k n) 1)
            ((even k) 2)
            (else 4)
        )
    )

    (define (simpson-iter func a k h)
        (if (> k n)
            0
            (+ (* (next k) (y func a k h)) (simpson-iter func a (+ k 1) h))
        )
    )

    (* (/ (h-func a b) 3) (simpson-iter f a 0 (h-func a b)))
)