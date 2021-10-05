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
        ( else (find-divisor n (next-divisor test-divisor)) ) ; 否则测试下一个数是否为因子
    )
)

(define (next-divisor n)
    (cond
        ((= n 2) 3)
        (else (+ n 2))
    )
)

(define (prime n)
    (= n (smallest-divisor n))
)