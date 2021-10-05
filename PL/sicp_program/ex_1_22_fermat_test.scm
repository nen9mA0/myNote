; ==== fast prime ====  (not used here)

(define (expmod base exp m)
    (cond
        ( (= exp 0) 1 )
        ( (even exp) (remainder (square (expmod base (/ exp 2) m)) m) )
        (else (remainder (* base (expmod base (- exp 1) m)) m))
    )
)

(define (fermat-test n)
    (define (try-it a)
        (= (expmod a n n) a)     ; 注意这里的判断是 a^n mod n == a，这是因为选取的a < n，所以必有 a mod n = a
    )
    (try-it (+ 1 (random (- n 1))))  ; 注意这里+1，是因为random n的返回值是0 ~ n-1，而需要的a满足 1 <= a <= n-1
)

(define (fast-prime n times)
    (cond
        ( (= times 0) true )
        ( (fermat-test n) (fast-prime n (- times 1)))
        (else false)
    )
)

(define (even n)
    (= (remainder n 2) 0)
)

; ==== prime =====

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

; ==== time prime test ====

(define (time-prime-test n)
    (newline)
    (display n)
    (start-prime-test n (real-time-clock))
)

(define (start-prime-test n start-time)
    (if (prime n)
        (report-prime (- (real-time-clock) start-time) n)
        (start-prime-test (next-odd n) start-time)
    )
)

(define (report-prime elapsed-time n)
    (display " *** ")
    (display elapsed-time)
    n
)

; ==== search for primes ====

(define (next-odd n)
    (if (= (remainder n 2) 1)
        (+ n 2)
        (+ n 1)
    )
)

(define (search-for-prime n)
    (time-prime-test n)
)
