# Chebyshev Polynomials

This calculator supports Chebyshev polynomial evaluation with real, complex, and WAV arguments.

## Function: `cheb(coeffs, x)`

Evaluates Chebyshev polynomial series at point x using Clenshaw algorithm.

**Parameters:**
- `coeffs` - Vector (row or column) containing coefficients
  - Row vector: `[(a0, a1, a2, ..., an)]` (recommended - easier to type)
  - Column vector: `[(a0); (a1); (a2); ...; (an)]`
- `x` - Real number, complex number, or WAV object

**Returns:** 
- Real or complex number for scalar arguments
- WAV object for WAV input (with automatic normalization if needed)

**Formula:**
    Result = a0·T0(x) + a1·T1(x) + a2·T2(x) + ... + an·Tn(x)

Where Tn(x) are Chebyshev polynomials of the first kind:
- T0(x) = 1
- T1(x) = x
- T2(x) = 2x² - 1
- T3(x) = 4x³ - 3x
- Recurrence: Tn+1(x) = 2x·Tn(x) - Tn-1(x)

---
## Examples

### Example 1: Simple Polynomial Evaluation

```
    ;; Define coefficients for T0 + 2·T1 + 3·T2 using row vector
    ;; T2(x) = 2x² - 1, so result = 1 + 2x + 3(2x² - 1) = 6x² + 2x - 2
    c := [(1, 2, 3)]
    
    ;; Evaluate at x = 0.5
    y := cheb(c, 0.5)
    ;; Result: 6·0.25 + 2·0.5 - 2 = 1.5 + 1 - 2 = 0.5
    
    ;; Plot the polynomial
    plot(cheb(c, t), -1, 1, t)
```

### Example 2: Understanding Waveshaping (DC Offset Issue)

```
    ;; IMPORTANT: cheb() does pointwise transformation, NOT filtering!
    
    ;; Linear transformation: 0.5 + 0.5·x
    c := [(0.5, 0.5)]
    
    ;; Input signal in range [-1, 1]
    w := afft([(440, 1); (880, 0.5)], 0.8)
    
    ;; Output will be in range [0, 1] - DC offset!
    shifted := cheb(c, w)
    
    ;; To see the issue:
    fftplot(w)        ;; Original: centered at 0
    fftplot(shifted)  ;; Shifted: centered at 0.5, same harmonics
    
    ;; For waveshaping without DC offset, use only odd terms
    c2 := [(0, 1)]  ;; T1(x) = x (identity, no change)
    unchanged := cheb(c2, w)
```

### Example 3: Adding Second Harmonic (Waveshaping)

```
    ;; T2(x) = 2x² - 1 generates second harmonic from sine
    ;; When x = sin(ωt): T2(sin ωt) = 2sin²(ωt) - 1 = -cos(2ωt)
    
    ;; Pure 220 Hz tone
    fundamental := afft([(220, 1)], 1)
    
    ;; Add second harmonic using T2
    ;; Mix: 90% fundamental + 10% second harmonic
    c := [(0, 0.9, 0.1)]  ;; 0·T0 + 0.9·T1 + 0.1·T2
    
    enriched := cheb(c, fundamental)
    
    ;; Verify: should see peaks at 220 Hz and 440 Hz
    h1 := fft(fundamental)
    h2 := fft(enriched)
    
    fftplot(fundamental)
    fftplot(enriched)
```

### Example 4: Third Harmonic Distortion

```
    ;; T3(x) = 4x³ - 3x generates third harmonic
    ;; Creates warm, tube-like distortion
    
    c := [(0, 1, 0, 0.2)]  ;; Add 20% of T3
    
    clean := afft([(440, 1)], 1)
    distorted := cheb(c, clean)
    
    ;; Original: only 440 Hz
    ;; Distorted: 440 Hz + 1320 Hz (third harmonic)
    fftplot(clean)
    fftplot(distorted)
```

### Example 5: Soft Clipping / Saturation

```
    ;; Smooth saturation curve prevents harsh clipping
    ;; Uses multiple Chebyshev terms for gradual compression
    
    ;; Soft saturation formula
    c := [(0, 0.9, 0, -0.15, 0, 0.02)]
    
    ;; Loud signal that would normally clip
    loud := afft([(440, 2)], 1)  ;; Amplitude > 1, will normalize
    
    ;; Apply soft clipping
    soft := cheb(c, loud)
    
    ;; Compare waveforms
    plot(ewav(loud, t), 0, 0.01, t)
    plot(ewav(soft, t), 0, 0.01, t)
    
    ;; Soft clipping adds harmonics
    fftplot(soft)
```

### Example 6: Symmetric Waveshaping (Odd Harmonics Only)

```
    ;; Use only odd Chebyshev terms (T1, T3, T5, ...)
    ;; Maintains symmetry, adds only odd harmonics
    
    ;; T1 + 0.3·T3 + 0.1·T5
    c := [(0, 1, 0, 0.3, 0, 0.1)]
    
    f0 := 110  ;; Low A
    clean := afft([(f0, 1)], 1)
    shaped := cheb(c, clean)
    
    ;; Should see: 110 Hz, 330 Hz (3rd), 550 Hz (5th)
    h := fft(shaped)
    fftplot(shaped)
```

### Example 7: Asymmetric Waveshaping (Even + Odd Harmonics)

```
    ;; Include even terms (T2, T4) for asymmetric distortion
    ;; Creates richer harmonic content
    
    c := [(0, 1, 0.2, 0.15, 0.05)]  ;; T1 + 0.2·T2 + 0.15·T3 + 0.05·T4
    
    clean := afft([(220, 1)], 1)
    shaped := cheb(c, clean)
    
    ;; Will have: 220, 440, 660, 880 Hz etc.
    fftplot(shaped)
```

### Example 8: Comparison - Linear vs Chebyshev

```
    ;; Show difference between power series and Chebyshev
    
    ;; Same numerical coefficients
    coeffs := [(0, 1, 0.2)]
    
    ;; Chebyshev: 0 + 1·T1(x) + 0.2·T2(x) = x + 0.2·(2x²-1) = 0.4x² + x - 0.2
    w := afft([(440, 1)], 1)
    cheb_result := cheb(coeffs, w)
    
    ;; Note: Chebyshev automatically handles the basis transformation
    ;; T2 term creates second harmonic at 880 Hz
    fftplot(cheb_result)
```

### Example 9: Tube Amplifier Emulation

```
    ;; Classic tube saturation curve using Chebyshev series
    ;; Emphasizes odd harmonics (warm sound)
    
    c := [(0, 0.95, 0, -0.2, 0, 0.08, 0, -0.02)]
    
    ;; Guitar-like signal
    guitar := afft([(196, 1); (196*3/2, 0.6); (196*2, 0.3)], 2)
    
    ;; Apply tube saturation
    tube := cheb(c, guitar)
    
    save("tube_sound.wav", tube)
    fftplot(tube)
```

### Example 10: Extreme Fuzz Effect

```
    ;; Heavy distortion with many harmonics
    
    c := [(0, 0.7, 0.3, 0.4, 0.2, 0.1)]
    
    clean := afft([(440, 1)], 1)
    fuzz := cheb(c, clean)
    
    ;; Very rich harmonic content
    h := fft(fuzz)
    fftplot(fuzz)
```

### Example 11: Complex Domain Analysis

```
    ;; Chebyshev polynomials extend to complex plane
    c := [(1, 0, 1)]  ;; T0 + T2 = 1 + 2x² - 1 = 2x²
    
    ;; Real evaluation
    y_real := cheb(c, 0.5)
    
    ;; Complex evaluation
    z := 0.5 + 0.3i
    y_complex := cheb(c, z)
    
    ;; Plot real and imaginary parts in complex domain
    plot(Re(cheb(c, t + 0.1i)), -1, 1, t)
    plot(Im(cheb(c, t + 0.1i)), -1, 1, t)
```

---

## Important Notes

### What cheb() Does

**Pointwise transformation:** Each sample is processed independently through the polynomial.

    ```output[i] = cheb(coeffs, input[i])```

**NOT frequency-domain filtering!** The function does not:
- Filter specific frequencies
- Implement low-pass/high-pass/band-pass filters
- Perform convolution

### What cheb() Is Good For

1. **Waveshaping / Distortion**
   - Adds harmonics based on Tn(sin ωt) property
   - Creates tube-like saturation
   - Soft clipping

2. **Function Approximation**
   - Polynomial evaluation with good numerical stability
   - Approximating expensive functions

3. **Harmonic Generation**
   - Tn(sin ωt) produces frequency n·ω
   - Controlled harmonic content for synthesis

### Avoiding DC Offset

To avoid DC offset in audio waveshaping:
- Use only odd terms: `[(0, a1, 0, a3, 0, a5, ...)]`
- First coefficient (T0 = constant) adds DC offset
- Even terms (T2, T4, ...) also shift the average

For symmetric distortion: `c := [(0, 1, 0, 0.3, 0, 0.1)]`