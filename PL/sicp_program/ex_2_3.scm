(define (edge-len segment)
    (let
        (
            (start (start-segment segment))
            (end (end-segment segment))
        )

        ( sqrt (+ (square (- (x-point start) (x-point end))) (square (- (y-point start) (y-point end)))) )
    )
)

(define (check-rect rect)
    (let
        (
            ( edge1 (make-segment (car (car rect)) (car (cdr rect))) )
            ( edge2 (make-segment (car (car rect)) (cdr (car rect))) )
            ( edge3 (make-segment (cdr (car rect)) (cdr (cdr rect))) )
            ( edge4 (make-segment (car (cdr rect)) (cdr (cdr rect))) )
        )

        (and (check-right edge1 edge2) (check-right edge2 edge3) (check-right edge3 edge4) (check-right edge1 edge4))
    )
)

(define (get-slope segment)
    (let
        (
            (start (start-segment segment))
            (end (end-segment segment))
        )

        (/ (- (y-point end) (y-point start)) (- (x-point end) (x-point start)))
    )
)

(define (check-right edge1 edge2)       ; check if an edge is at right angles to the other edge
    (let
        (
            ( k1 (get-slope edge1) )
            ( k2 (get-slope edge2) )
        )

        (= (* k1 k2) -1)
    )
)

(define (make-rect w x y z)
    (cons (cons w x) (cons y z))
)

(define (make-rect-point w x y z)       ; build rectangle from four points
    (let
        (
            (rect (make-rect w x y z))
        )

        (if (check-rect rect)
            rect
            (error "input cannot build a rectangle")
        )
    )
)

(define (make-rect-segment a b)            ; build rectangle from two diagonals
    (let
        (
            (rect (make-rect (start-segment a) (start-segment b) (end-segment a) (end-segment b)))
        )

        (if (check-rect rect)
            rect
            (error "input cannot build a rectangle")
        )
    )
)

(define (rect-area rect)
    (let
        (
            ( edge1 (make-segment (car (car rect)) (car (cdr rect))) )
            ( edge2 (make-segment (car (car rect)) (cdr (car rect))) )
        )

        (* (edge-len edge1) (edge-len edge2))
    )
)

(define (rect-perimeter rect)
    (let
        (
            ( edge1 (make-segment (car (car rect)) (car (cdr rect))) )
            ( edge2 (make-segment (car (car rect)) (cdr (car rect))) )
        )

        (* (+ (edge-len edge1) (edge-len edge2)) 2)
    )
)


; (define (test-point)      ; test the program
;     (define w (make-point 1 2))
;     (define x (make-point 5 4))
;     (define y (make-point 0 4))
;     (define z (make-point 3 6))

;     (let
;         (
;             (rect (make-rect-point w x y z))
;         )

;         (newline)
;         (display (rect-area rect))
;         (display (rect-perimeter rect))
;     )
; )

(define (test-point)      ; test the program
    (define w (make-point 1 2))
    (define x (make-point 5 4))
    (define y (make-point 0 4))
    (define z (make-point 4 6))

    (let
        (
            (rect (make-rect-point w x y z))
        )

        (newline)
        (display (rect-area rect))
        (newline)
        (display (rect-perimeter rect))
    )
)

(define (test-segment)
    (define sa (make-point 1 2))
    (define ea (make-point 0 4))
    (define sb (make-point 5 4))
    (define eb (make-point 4 6))

    (let
        (
            (rect (make-rect-segment (make-segment sa ea) (make-segment sb eb)))
        )

        (newline)
        (display (rect-area rect))
        (newline)
        (display (rect-perimeter rect))
    )
)