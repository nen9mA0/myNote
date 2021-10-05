(define (cons x y)
    (define (dispatch m)
        (cond
            ( (= m 0) x)
            ( (= m 1) y)
            ( else (error "Argument must be 0 or 1") )
        )
    )
    dispatch		; 注意这里返回的是一个函数，其定义在cons内
)

(define (car m)
    (m 0)
)

(define (cdr m)
    (m 1)
)