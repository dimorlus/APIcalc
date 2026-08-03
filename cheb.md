# Chebyshev Polynomials in Calculator

This calculator supports Chebyshev polynomial evaluation with real, complex, and WAV arguments.

## Function: `cheb(coeffs, x)`

Evaluates Chebyshev polynomial series at point x using Clenshaw algorithm.

**Parameters:**
- `coeffs` - Column vector (matrix with 1 column) containing coefficients [a0; a1; a2; ...; an]
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
    ;; Define coefficients for T0 + 2·T1 + 3·T2
    ;; T2(x) = 2x² - 1, so result = 1 + 2x + 3(2x² - 1) = 6x² + 2x - 2
    c := [1; 2; 3]
    
    ;; Evaluate at x = 0.5
    y := cheb(c, 0.5)
    ;; Result: 6·0.25 + 2·0.5 - 2 = 1.5 + 1 - 2 = 0.5
    
    ;; Plot the polynomial
    plot(cheb(c, t), -1, 1, t)

### Example 2: Low-Pass Filter Approximation
    ;; Chebyshev approximation for smooth low-pass filter
    ;; Keeps DC (x=1), attenuates high frequencies
    c := [0.5; 0.5; 0; 0; 0]
    
    ;; Generate test signal (440 Hz + harmonic at 880 Hz)
    w := play(sin(t*2*pi*440) + 0.5*sin(t*2*pi*880), 0, 1, t)
    
    ;; Apply Chebyshev filter
    filtered := cheb(c, w)
    
    ;; Compare original and filtered
    fftplot(w)
    fftplot(filtered)
    
    ;; Save result
    save("filtered.wav", filtered)

### Example 3: Waveshaping / Distortion
    ;; Non-linear waveshaping using Chebyshev polynomials
    ;; Creates harmonic distortion
    
    ;; Mild distortion: adds 3rd harmonic
    ;; T3(x) = 4x³ - 3x
    c := [0; 1; 0; 0.3]
    
    ;; Generate clean sine wave
    clean := play(sin(t*2*pi*220), 0, 2, t)
    
    ;; Apply waveshaping
    distorted := cheb(c, clean)
    
    ;; Analyze harmonics
    h_clean := fft(clean)
    h_dist := fft(distorted)
    
    ;; Visualize
    fftplot(clean)
    fftplot(distorted)

### Example 4: Complex Domain Evaluation
    ;; Chebyshev polynomials extend to complex plane
    c := [1; 0; 1]  ;; T0 + T2 = 1 + 2x² - 1 = 2x²
    
    ;; Real evaluation
    y_real := cheb(c, 0.5)
    
    ;; Complex evaluation
    z := 0.5 + 0.3i
    y_complex := cheb(c, z)
    
    ;; Plot real and imaginary parts
    plot(Re(cheb(c, t + 0.1i)), -1, 1, t)
    plot(Im(cheb(c, t + 0.1i)), -1, 1, t)

### Example 5: Approximating Functions
    ;; Chebyshev polynomials provide near-optimal polynomial approximations
    ;; Example: approximate sin(πx/2) on [-1, 1]
    
    ;; Chebyshev coefficients for sin(πx/2) (computed externally)
    c := [0; 1.5708; 0; -0.6459; 0; 0.0796]
    
    ;; Compare approximation with actual function
    plot(sin(t*pi/2), -1, 1, t)
    plot(cheb(c, t), -1, 1, t)
    
    ;; Error plot
    plot(sin(t*pi/2) - cheb(c, t), -1, 1, t)

### Example 6: Audio Equalization
    ;; Multi-band emphasis using Chebyshev polynomial
    
    ;; Coefficients for midrange boost
    c := [0.8; 0; 0.4; 0; -0.2]
    
    ;; Load audio file
    audio := load("music.wav")
    
    ;; Apply equalization
    eq_audio := cheb(c, audio)
    
    ;; Save processed audio
    save("music_eq.wav", eq_audio)
    
    ;; Compare spectra
    fftplot(audio)
    fftplot(eq_audio)

### Example 7: Soft Clipping
    ;; Smooth saturation curve using Chebyshev series
    ;; Prevents hard clipping while adding harmonics
    
    ;; Coefficients for soft saturation
    c := [0; 0.9; 0; -0.1; 0; 0.01]
    
    ;; Generate loud signal that would clip
    loud := play(2*sin(t*2*pi*440), 0, 1, t)
    
    ;; Apply soft clipping
    soft_clip := cheb(c, loud)
    
    ;; Compare waveforms
    plot(ewav(loud, t), 0, 0.01, t)
    plot(ewav(soft_clip, t), 0, 0.01, t)

### Example 8: Harmonic Generation
    ;; Generate specific harmonic content
    ;; Tn(sin(ωt)) produces frequency nω
    
    ;; Add 2nd and 4th harmonics to fundamental
    c := [0; 1; 0.3; 0; 0.1]
    
    ;; Pure 220 Hz sine
    fundamental := play(sin(t*2*pi*220), 0, 2, t)
    
    ;; Add harmonics at 440 Hz and 880 Hz
    rich := cheb(c, fundamental)
    
    ;; Verify harmonics
    h := fft(rich)
    ;; Should show peaks at 220, 440, and 880 Hz
    
    fftplot(rich)

---

## Technical Details

### Numerical Stability

The function uses the **Clenshaw algorithm**, which is numerically stable even for high-degree polynomials:

    b[n+1] = b[n+2] = 0
    b[k] = a[k] + 2·x·b[k+1] - b[k+2]  for k = n-1, n-2, ..., 0
    Result = a[0] + x·b[1] - b[2]

This avoids direct computation of individual Chebyshev polynomials and reduces rounding errors.

### Complex Arguments

For complex x = re + im·i:
- Uses complex arithmetic throughout Clenshaw recursion
- Returns complex result
- Automatically switches to fast real path if imaginary part is zero

### WAV Processing

**Two-Pass Algorithm:**

1. **First pass:** Evaluate polynomial at all samples, find max(|y|)
2. **Normalization decision:**
   - If max(|y|) > 1.0 → scale all samples by 1/max(|y|)
   - If max(|y|) ≤ 1.0 → keep original values
3. **Second pass:** Write normalized samples to output WAV

**Why normalize?**
- WAV samples must stay in range [-1, 1] (or [-32768, 32767] for 16-bit)
- Polynomial evaluation can produce values outside this range
- Automatic normalization prevents clipping while preserving waveform shape

### Chebyshev Properties

**Orthogonality on [-1, 1]:**
    ∫ Tn(x)·Tm(x) / √(1-x²) dx = 0  for n ≠ m

**Minimax property:**
- Chebyshev polynomials minimize maximum approximation error
- Best polynomial approximation for many functions

**Recurrence relation:**
    T0(x) = 1
    T1(x) = x
    Tn+1(x) = 2x·Tn(x) - Tn-1(x)

**Trigonometric form:**
    Tn(cos θ) = cos(nθ)

---

## Use Cases

### Signal Processing
- **Filtering:** Design filters with specific frequency response
- **Equalization:** Boost or attenuate frequency ranges
- **Waveshaping:** Non-linear distortion for audio effects

### Function Approximation
- **Fast evaluation:** Replace expensive functions with polynomial
- **Near-optimal:** Chebyshev approximations minimize max error

### Audio Synthesis
- **Harmonic control:** Tn(sin ωt) generates frequency nω
- **Additive synthesis:** Build complex timbres from harmonics

### Digital Effects
- **Saturation:** Soft clipping curves
- **Exciter:** Add artificial harmonics
- **Character:** Analog-style non-linearity

---

## Comparison with Power Polynomials

Given coefficients c = [a0; a1; a2]:

**Power polynomial:**  a0 + a1·x + a2·x²

**Chebyshev polynomial:**  a0·T0(x) + a1·T1(x) + a2·T2(x) = a0 + a1·x + a2·(2x² - 1)

**Advantages of Chebyshev:**
- Better numerical stability for high degrees
- More uniform error distribution
- Easier to control harmonic content in audio

---

## Future Enhancements

Planned additions:
- Chebyshev polynomial coefficient computation from function samples
- Type-II Chebyshev polynomials (Un)
- Automatic best-fit approximation for arbitrary functions
- Filter design tools using Chebyshev response