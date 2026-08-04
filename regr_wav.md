# Polynomial and Regression Functions for WAV Processing

The calculator's regression evaluation functions can process WAV files pointwise, applying mathematical transformations to audio signals.

## Available Functions

- `clcpoly(coeffs, wav)` - Polynomial evaluation (any degree up to 6)
- `clcexp(coeffs, wav)` - Exponential function: y = a·exp(b·x)
- `clclog(coeffs, wav)` - Logarithmic function: y = a + b·ln(x)
- `clcpow(coeffs, wav)` - Power function: y = a·x^b
- `clcinv(coeffs, wav)` - Inverse function: y = a + b/x

**Note:** All functions work pointwise - each sample is transformed independently. Automatic normalization is applied if output exceeds [-1, 1] range.

---

## Examples

### Example 1: Polynomial Waveshaping (Power Series)

```
    ;; Use polynomial for smooth waveshaping
    ;; P(x) = x + 0.2x² + 0.1x³ (adds harmonics)
    
    ;; Coefficients from highest to lowest degree: [a3, a2, a1, a0]
    poly := [(0.1, 0.2, 1, 0)]
    
    ;; Generate clean tone
    clean := afft([(440, 1)], 1)
    
    ;; Apply polynomial waveshaping
    shaped := clcpoly(poly, clean)
    
    ;; Compare
    fftplot(clean)
    fftplot(shaped)  ;; Shows additional harmonics
    
	;; Short form
	fftplot(clcpoly([(0.1, 0.2, 1, 0)], afft([(440, 1)], 1)))
	
    save("poly_shaped.wav", shaped)
```

### Example 2: Soft Clipping with Polynomial

```
    ;; Cubic soft clipper: y = x - 0.33x³
    ;; Maps [-∞, ∞] → [-1, 1] smoothly
    
    poly := [(-0.33, 0, 1, 0)]  ;; -0.33x³ + x
    
    ;; Create loud signal
    loud := afft([(440, 2)], 1)  ;; Amplitude 2 (will normalize)
    
    ;; Apply soft clipping
    clipped := clcpoly(poly, loud)
    
    ;; Compare waveforms
    plot(ewav(loud, t), 0, 0.01, t)
    plot(ewav(clipped, t), 0, 0.01, t)
```

### Example 3: Exponential Envelope Shaping

```
    ;; Apply exponential decay/attack shape
    ;; y = a·exp(b·x) where x ∈ [-1, 1]
    
    ;; Parameters: [b, a]
    ;; b > 0: emphasizes positive values
    ;; b < 0: emphasizes negative values
    
    exp_up := [(1, 1)]      ;; Exponential rise
    exp_down := [(-1, 1)]   ;; Exponential fall
    
    sig := afft([(440, 1)], 1)
    
    ;; Apply exponential shaping
    shaped_up := clcexp(exp_up, sig)
    shaped_down := clcexp(exp_down, sig)
    
    fftplot(shaped_up)
    fftplot(shaped_down)
```

### Example 4: Dynamic Range Compression (Power Law)

```
    ;; Compress dynamic range using y = a·x^b
    ;; b < 1: compression (soft sounds louder)
    ;; b > 1: expansion (emphasize loud sounds)
    
    ;; Parameters: [b, a]
    compress := [(0.5, 1)]   ;; Square root compression
    expand := [(2, 1)]       ;; Squaring expansion
    
    ;; Load audio
    audio := load("music.wav")
    
    ;; Apply compression
    compressed := clcpow(compress, audio)
    expanded := clcpow(expand, audio)
    
    save("compressed.wav", compressed)
    save("expanded.wav", expanded)
    
    ;; Compare dynamics
    fftplot(audio)
    fftplot(compressed)
```

### Example 5: Logarithmic Compression (Mu-Law Style)

```
    ;; Logarithmic characteristic: y = a + b·ln(|x| + ε)
    ;; Similar to mu-law/A-law companding
    
    ;; Parameters: [b, a]
    ;; Note: ln(x) requires x > 0, so this works best with abs() or offset
    
    log_compress := [(0.5, 0)]
    
    sig := afft([(440, 1)], 1)
    
    ;; Apply log compression
    compressed := clclog(log_compress, sig)
    
    fftplot(compressed)
```

### Example 6: Inverse Function (Proximity Effect)

```
    ;; y = a + b/x creates emphasis near zero
    ;; Can simulate proximity effect in microphones
    
    ;; Parameters: [b, a]
    proximity := [(0.3, 1)]
    
    vocal := load("vocals.wav")
    
    ;; Apply proximity effect
    boomy := clcinv(proximity, vocal)
    
    save("boomy_vocals.wav", boomy)
    
    ;; Compare frequency content
    fftplot(vocal)
    fftplot(boomy)
```

### Example 7: Symmetric Polynomial (Odd Powers Only)

```
    ;; Maintains signal symmetry (no DC offset)
    ;; Only odd powers: x, x³, x⁵
    
    ;; y = x - 0.2x³ + 0.05x⁵
    poly := [(0.05, 0, -0.2, 0, 1, 0)]
    
    clean := afft([(220, 1)], 2)
    
    distorted := clcpoly(poly, clean)
    
    ;; Only odd harmonics: 220, 660, 1100 Hz
    h := fft(distorted)
    fftplot(distorted)
```

### Example 8: Tube Saturation (Exponential)

```
    ;; Simulate tube amplifier saturation
    ;; Uses exponential to create soft limiting
    
    ;; Mild saturation
    tube := [(0.8, 1.2)]  ;; y = 1.2·exp(0.8·x)
    
    guitar := afft([(196, 1); (196*1.5, 0.6); (196*2, 0.3)], 2)
    
    saturated := clcexp(tube, guitar)
    
    save("tube_guitar.wav", saturated)
    fftplot(saturated)
```

### Example 9: Asymmetric Clipping (Power Function)

```
    ;; Different compression for positive/negative
    ;; Creates even harmonics (asymmetry)
    
    ;; y = sign(x) · |x|^0.7
    ;; Approximated with power function
    
    asymmetric := [(0.7, 1)]
    
    bass := afft([(110, 1)], 2)
    
    clipped := clcpow(asymmetric, bass)
    
    ;; Should show both odd and even harmonics
    fftplot(clipped)
```

### Example 10: Combining Multiple Functions

```
    ;; Chain multiple transformations
    
    ;; 1. Start with clean signal
    sig := afft([(440, 1)], 2)
    
    ;; 2. Polynomial waveshaping (add harmonics)
    poly := [(0, 0.15, 1, 0)]
    step1 := clcpoly(poly, sig)
    
    ;; 3. Exponential compression
    comp := [(0.6, 1)]
    step2 := clcexp(comp, step1)
    
    ;; 4. Final soft clipping
    clip := [(0, -0.1, 1, 0)]  ;; x - 0.1x²
    final := clcpoly(clip, step2)
    
    ;; Compare all stages
    fftplot(sig)
    fftplot(step1)
    fftplot(step2)
    fftplot(final)
    
    save("multi_stage.wav", final)
```

### Example 11: High-Degree Polynomial Distortion

```
    ;; Complex harmonic structure using high-degree polynomial
    
    ;; 6th degree: includes many harmonics
    poly := [(0.01, -0.02, 0.05, -0.1, 0.2, 1, 0)]
    
    tone := afft([(440, 1)], 1)
    
    complex_dist := clcpoly(poly, tone)
    
    ;; Rich harmonic spectrum
    h := fft(complex_dist)
    fftplot(complex_dist)
```

### Example 12: Inverse Function for Nulling Effect

```
    ;; y = a + b/x creates interesting nulls near zero crossings
    
    ;; Small b creates subtle effect
    inv := [(0.1, 1)]
    
    sig := afft([(440, 1); (880, 0.5)], 1)
    
    modified := clcinv(inv, sig)
    
    ;; Notice how zero crossings are affected
    plot(ewav(sig, t), 0, 0.01, t)
    plot(ewav(modified, t), 0, 0.01, t)
```

---

## Technical Notes

### Input Range
All functions receive normalized samples in range [-1, 1] from WAV (16-bit samples divided by 32768).

### Output Normalization
- **First pass:** Computes all output samples, finds max(|y|)
- **If max > 1.0:** Scales all samples by 1/max
- **If max ≤ 1.0:** Keeps original values
- This prevents clipping while preserving waveform shape

### Function Characteristics

**Polynomial (clcpoly):**
- Most flexible: can create any smooth curve
- Even powers → even harmonics
- Odd powers → odd harmonics

**Exponential (clcexp):**
- Natural saturation curves
- Good for tube/tape emulation
- Creates soft compression

**Logarithmic (clclog):**
- Companding (compression/expansion)
- Similar to mu-law encoding
- Careful with negative values!

**Power (clcpow):**
- Dynamic range control
- b < 1: compression
- b > 1: expansion
- b = 0.5: square root (classic compressor)

**Inverse (clcinv):**
- Creates singularity near x=0
- Good for proximity effects
- Extreme near zero crossings

### Comparison with Chebyshev

| Feature | Polynomial (clcpoly) | Chebyshev (cheb) |
|---------|---------------------|------------------|
| Basis | Power series (x^n) | Chebyshev polynomials (Tn) |
| Harmonics | Direct control via powers | Tn(sin ωt) = harmonic nω |
| Stability | Can be unstable for high degrees | More numerically stable |
| Use case | General waveshaping | Controlled harmonic generation |

---

## Practical Applications

### Audio Effects
- **Saturation/Distortion:** Polynomial, exponential
- **Compression:** Power function with b < 1
- **Expansion:** Power function with b > 1
- **Soft Clipping:** Polynomial with odd powers
- **Tube Emulation:** Exponential or polynomial

### Signal Conditioning
- **Dynamic range:** clcpow for compression/expansion
- **Emphasis:** clcinv for bass boost near silence
- **Smoothing:** Low-degree polynomial

### Creative Effects
- **Harmonic generation:** High-degree polynomials
- **Asymmetric distortion:** Even-order terms
- **Complex textures:** Chain multiple functions