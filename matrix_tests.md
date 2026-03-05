# Matrix Test Cases

## Setup
```
A := [(1,2,3);(4,5,6);(7,8,9)]       ;; 3x3 singular (det=0)
B := [(10,11,12);(13,14,15);(16,17,18)] ;; 3x3
C := [(1,2);(3,4);(5,6)]             ;; 3x2
D := [(1,2);(3,4)]                   ;; 2x2 invertible (det=-2)
```

## Binary: matrix + matrix (same dimensions)
```
A+B  -> [(11,13,15);(17,19,21);(23,25,27)]
A-B  -> [(-9,-9,-9);(-9,-9,-9);(-9,-9,-9)]
```

## Binary: matrix op scalar
```
A+2  -> [(3,4,5);(6,7,8);(9,10,11)]
2+A  -> [(3,4,5);(6,7,8);(9,10,11)]
A-2  -> [(-1,0,1);(2,3,4);(5,6,7)]
2-A  -> [(1,0,-1);(-2,-3,-4);(-5,-6,-7)]
A*2  -> [(2,4,6);(8,10,12);(14,16,18)]
2*A  -> [(2,4,6);(8,10,12);(14,16,18)]
A/2  -> [(0.5,1,1.5);(2,2.5,3);(3.5,4,4.5)]
2/A  -> element-wise: [(2,1,0.6667);(0.5,0.4,0.3333);(0.2857,0.25,0.2222)]
```

## Matrix multiply (true matrix multiplication)
```
A*B  -> [(84,90,96);(201,216,231);(318,342,366)]
A*C  -> [(22,28);(49,64);(76,100)]     ;; 3x3 * 3x2 = 3x2
C*A  -> ERROR: dimension mismatch (3x2 * 3x3, inner 2 != 3)
```

## Unary
```
-A   -> [(-1,-2,-3);(-4,-5,-6);(-7,-8,-9)]
~A   -> [(1,4,7);(2,5,8);(3,6,9)]         ;; transpose
~C   -> [(1,3,5);(2,4,6)]                 ;; 3x2 -> 2x3
```

## Power (integer, non-negative, square matrix only)
```
D^0  -> [(1,0);(0,1)]                     ;; identity
D^1  -> [(1,2);(3,4)]
D^2  -> [(7,10);(15,22)]
D^3  -> [(37,54);(81,118)]
D^-1 -> ERROR: negative exponent not allowed
D^1.5-> ERROR: non-integer exponent not allowed
C^2  -> ERROR: matrix power requires square matrix
```

## Inverse (! operator)
```
!D   -> [(-2,1);(1.5,-0.5)]
D*!D -> [(1,0);(0,1)]                     ;; identity (verify)
!A   -> ERROR: singular matrix (det=0)
!C   -> ERROR: matrix must be square
```

## Error cases
```
A+C  -> ERROR: dimension mismatch (3x3 + 3x2)
A/B  -> ERROR: matrix/matrix division not defined
```

## Scalar results (functions — not yet implemented)
```
tr(A)   -> 15                             ;; trace = sum of diagonal
tr(D)   -> 5
det(D)  -> -2
det(A)  -> 0                              ;; singular!
norm(A) -> 16.881943016134134             ;; Frobenius norm
norm(D) -> 5.477225575051661
```
