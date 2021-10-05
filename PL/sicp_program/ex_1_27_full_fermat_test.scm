; ==== fermat prime ====

(define (expmod base exp m)
    (cond
        ( (= exp 0) 1 )
        ( (even exp) (remainder (square (expmod base (/ exp 2) m)) m) )
        (else (remainder (* base (expmod base (- exp 1) m)) m))
    )
)

(define (fermat-test a n)
    (= (expmod a n n) a)
)

(define (fermat-iter a n)
    (cond
        ( (= a 1) true )
        ( (fermat-test a n) (fermat-iter (- a 1) n) )
        (else false)
    )
)

(define (fermat-prime n)
    (fermat-iter (- n 1) n)
)

(define (even n)
    (= (remainder n 2) 0)
)

; ==== prime ====
(define (smallest-divisor n)
    (find-divisor n 2)   ; 从2开始测试
)

(define (divide a b)
    (= (remainder a b) 0)
)

(define (find-divisor n test-divisor)
    (cond
        ( (> (square test-divisor) n)  n )           ; 若test-divisor^2 > n，说明没有找到因子，这里直接返回n
        ( (divide n test-divisor)  test-divisor )    ; 若能被整除，则找到因子
        ( else (find-divisor n (+ test-divisor 1)) ) ; 否则测试下一个数是否为因子
    )
)

(define (prime n)
    (= n (smallest-divisor n))
)