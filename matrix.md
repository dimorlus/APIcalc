### Matrix Support

Matrices up to 7×7 are supported. Matrix syntax:
```
[(a11, a12, ...); (a21, a22, ...); ...]
```
Each row is enclosed in `()`, rows are separated by `;`, the whole matrix is enclosed in `[]`. Elements can be plain numbers (SI suffixes supported) or expressions using previously defined constants and variables, as long as the result is a real number or integer. Complex numbers, strings, or matrices cannot be matrix elements. Variables declared inside the matrix are local to it and not accessible afterwards:
```
[(-1k, 2m, 3M); (4, 5u, 6n); (7p, 8G, 9)]
[(a:=2*pi, a/3); (b:=e, b/2)]  →  [(6.283, 2.094); (2.718, 1.359)]
```
When the result is a matrix, it is displayed in engineering format, one row per line:
```
[(     1,      2,      3);
 (     4,      5,      6);
 (     7,      8,      9)]
```
Matrix elements can be accessed by index [row, col] for both reading and writing. Rows and cols are zero-based.
```
A := [(1,2,3);(4,5,6);(7,8,9)]
A[1,1] → 5
A[1,1] := 10;A → [(1,2,3);(4,10,6);(7,8,9)]
```
Access to a matrix element by index is possible both by row and column, and by a single through index.
```
[(1, 2, 3); (4, 5, 6); (7, 8, 9)][1,1] → 5
[(1, 2, 3); (4, 5, 6); (7, 8, 9)][4] → 5
```
Elements that are negligibly small compared to the matrix norm (Frobenius) are displayed as zero to suppress numerical noise. Using index access, you can view the unfiltered value of an element.
```
[(-1k, 2m, 3M); (4, 5u, 6n); (7p, 8G, 9)] → [(-1k, 0, 3M); (0, 0, 0); (0, 8G, 9)] ;;Filtered
[(-1k, 2m, 3M); (4, 5u, 6n); (7p, 8G, 9)][1,1] → 5e-06                            ;;Raw
```

#### Matrix Operations

**Binary operators** (where `M` = matrix, `s` = scalar):

| Expression         | Result | Notes                                 |
| ------------------ | ------ | ------------------------------------- |
| `M + M`            | matrix | element-wise, dimensions must match   |
| `M - M`            | matrix | element-wise                          |
| `M * M`            | matrix | true matrix multiplication            |
| `M + s`, `s + M`   | matrix | scalar added to each element          |
| `M - s`, `s - M`   | matrix |                                       |
| `M * s`, `s * M`   | matrix | scalar multiplication                 |
| `M / s`            | matrix | divide each element by scalar         |
| `s / M`            | matrix | divide scalar by each element         |
| `M ^ n`            | matrix | integer power n≥0, square matrix only |
| `M == M`, `M != M` | 0 or 1 | all elements equal?                   |
| `M // s`, `M // M` | matrix | parallel resistors, element-wise      |

**Unary operators:**

| Expression | Result | Notes                               |
| ---------- | ------ | ----------------------------------- |
| `-M`       | matrix | negate all elements                 |
| `~M`       | matrix | transpose (rows ↔ columns)          |
| `!M`       | matrix | matrix inverse (square matrix only) |

#### Matrix Functions

| Function           | Returns | Notes                                                        |
|--------------------|---------| -------------------------------------------------------------|
| `tr(M)`            | scalar  | trace — sum of diagonal elements                             |
| `det(M)`           | scalar  | determinant, square matrix only                              |
| `norm(M)`          | scalar  | Frobenius norm √(Σ aᵢⱼ²)                                     |
| `abs(M)`           | matrix  | element-wise absolute value                                  |
| `dot(A, B)`        | scalar  | dot product of two vectors (1×N or N×1), any length          |
| `cross(A, B)`      | vector  | cross product of two 3-element vectors (1×3 or 3×1)          |
| `rows(M)`          | scalar  | Return rows of matrix                                        |
| `cols(M)`          | scalar  | Return columns of matrix                                     |
| `size(M)`          | scalar  | Return rows\*columns of matrix                               |
| `zeros(r,c)`       | matrix  | Return zero matrix with r rows and c columns                 |
|`diag(r,c),eye(r,c)`| matrix  | Return zero matrix with r rows and c columns with 1 diagonal |

#### Matrix Examples

```
A := [(1,2,3);(4,5,6);(7,8,9)]
D := [(1,2);(3,4)]

A + 2            →  [(3,4,5);(6,7,8);(9,10,11)]
A * 2            →  [(2,4,6);(8,10,12);(14,16,18)]
~A               →  [(1,4,7);(2,5,8);(3,6,9)]     ;; transpose
D^2              →  [(7,10);(15,22)]
!D               →  [(-2,1);(1.5,-0.5)]           ;; inverse
D * !D           →  [(1,0);(0,1)]                 ;; identity
tr(A)            →  15
det(D)           →  -2
norm(D)          →  5.477225575051661

;; vectors (row or column):
dot([(1,2,3)], [(4,5,6)])          			→  32
dot([(3,4)], [(3,4)])              			→  25     ;; = norm^2
cross([(1,0,0)], [(0,1,0)])        			→  [(0,0,1)]
cross([(1,2,3)], [(4,-1,2)])       			→  [(7,10,-9)]
dot(cross([(1,2,3)],[(4,-1,2)]), [(1,2,3)]) → 0  ;; cross product ⊥ both inputs

r:=3;A:=zeros(r,r);for(A[n/r,n%r]:=n;A, 0, r*r-1, n) → [(0, 1, 2); (3, 4, 5); (6, 7, 8)]

F1:=[(10, 5, 0)]; F2:=[(0, 10, 5)];degr:=angle(F1, F2)/deg → 66.42182152179818
V_sns:=[(10); (0)]; ang:=pi/4;V_global:= rot2(ang) V_sns → [(7.071); (7.071)]
```

$$\begin{cases} 2x - 3y = -4 \\\\ 3x - 2y = -1 \end{cases} \Rightarrow \begin{pmatrix} x \\\\ y \end{pmatrix} = \begin{pmatrix} 1 \\\\ 2 \end{pmatrix}$$
```
;;From consts.txt:
;; Solves a system of linear equations A*X = B (returns vector X)
{solve_lin(A, B) !A * B}

solve_lin([(2, -3);(3, -2)],[(-4);(-1)]) → [(1); (2)]
```
