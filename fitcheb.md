# Chebyshev Polynomial Regression

## Function: `regrcheb(filename, mask, degree)`

Fits Chebyshev polynomial to data points from file using least squares method.

**Parameters:**
- `filename` - Text file with data points (x, y pairs)
- `mask` - Format mask for parsing (e.g., "ff" for two floats)
- `degree` - Polynomial degree (1 to 6)

**Returns:** Row vector with Chebyshev coefficients [c0, c1, c2, ..., cn]

**Important:** Data is automatically normalized to [-1, 1] range before fitting.

---

## Example

### Step 1: Create test data file

Create file `data.txt` with noisy sine wave:
```
0.0   0.05 
0.5   0.52 
1.0   0.88 
1.5   0.98 
2.0   0.92 
2.5   0.61 
3.0   0.18 
3.5  -0.35 
4.0  -0.72 
4.5  -0.95 
5.0  -0.98 
5.5  -0.85 
6.0  -0.32
```

### Step 2: Fit Chebyshev polynomial
```
;; Fit 3rd degree Chebyshev polynomial 
c := regrcheb("data.txt", "ff", 3)
;; Result: 
c = [(c0, c1, c2, c3)] ;; Example: c ≈ [(0.05, 0.95, 0.01, -0.15)]
```

### Step 3: Evaluate fitted function

**Problem:** Coefficients are for normalized x ∈ [-1, 1]

You need to manually normalize your x values:

```
;; Original data range: x ∈ [0, 6] 
;; Normalize formula: 
x_norm = 2*(x - x_min)/(x_max - x_min) - 1
x_min := 0 
x_max := 6
;; To evaluate at x = 2.5: 
x := 2.5 
x_norm := 2*(x - x_min)/(x_max - x_min) - 1  ;; = -0.1667
;; Evaluate Chebyshev polynomial 
y := cheb(c, x_norm)
```

### Step 4: Plot comparison
```
;; 1. Generate test file programmatically (in external tool) 
;;    data.txt contains: x, sin(x) with some noise
;; 2. Fit different degrees 
c3 := regrcheb("data.txt", "ff", 3) 
c4 := regrcheb("data.txt", "ff", 4) 
c5 := regrcheb("data.txt", "ff", 5)
;; 3. Define range (must match your data file!) 
x_min := 0 x_max := 6.28  ;; ~2π
;; 4. Normalization helper 
normalize(x) := 2*(x - x_min)/(x_max - x_min) - 1
;; 5. Evaluate fitted functions 
y3 := cheb(c3, normalize(3.14)) 
y4 := cheb(c4, normalize(3.14)) 
y5 := cheb(c5, normalize(3.14))
;; 6. Plot fitted curves 
plot(cheb(c3, normalize(t)), x_min, x_max, t) 
plot(cheb(c4, normalize(t)), x_min, x_max, t) 
plot(cheb(c5, normalize(t)), x_min, x_max, t)
```