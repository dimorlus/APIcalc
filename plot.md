# Plotting Functions Reference

This document describes all plotting functions available in the calculator for visualizing mathematical functions and circuit characteristics.

## Table of Contents

1. [Cartesian Plots](#cartesian-plots)
2. [Polar Plots](#polar-plots)
3. [Parametric Plots](#parametric-plots)
4. [Logarithmic Plots](#logarithmic-plots)
5. [Smith Chart](#smith-chart)
6. [Common Parameters](#common-parameters)
7. [Examples](#examples)

---

## Cartesian Plots

Standard $y = f(x)$ plots in Cartesian coordinates.

### Functions

| Function | Description |
|----------|-------------|
| `plot(expr, from, to, var)` | Display plot on screen |
| `fplot(filename, expr, from, to, var)` | Save plot to file |
| `oplot(filename, expr, from, to, var)` | Overlay plot on existing file |

### Syntax
`plot(expression, start_value, end_value, variable_name)`

### Parameters

- **expr**: Mathematical expression (function of variable)
- **from**: Starting value of the variable
- **to**: Ending value of the variable
- **var**: Variable name (typically `x`, `t`, `f`, etc.)
- **filename**: Output file path (for `fplot` and `oplot`)

### Features

- Automatic axis scaling
- Grid with 10 divisions
- Zero axes marked in different color
- Automatic range padding (10%)

### Examples
```
;; Simple sine wave 
plot(sin(x), 0, 2*pi, x)

;; Damped oscillation 
plot(exp(-t/10)*sin(t), 0, 50, t)

;; Transfer function magnitude 
fplot("filter.bmp", abs(1/(1+i2 pi*f/1k)), 1, 100k, f)

;; Overlay multiple functions 
fplot("combined.bmp", sin(x), 0, 2 pi, x) 
oplot("combined.bmp", cos(x), 0, 2 pi, x)
```
---

## Polar Plots

Plots in polar coordinates where $r = f(\theta)$.

### Functions

| Function | Description |
|----------|-------------|
| `plotpol(expr, from, to, var)` | Display polar plot on screen |
| `fplotpol(filename, expr, from, to, var)` | Save polar plot to file |
| `oplotpol(filename, expr, from, to, var)` | Overlay polar plot on existing file |

### Syntax

`plotpol(r_expression, angle_start, angle_end, angle_variable)`

### Features

- Circular grid with radial divisions
- Angular grid lines (every 30°)
- Center always at plot center
- Automatic radius scaling

### Common Polar Functions

#### Rose Curves
```
;; 5-petal rose 
plotpol(5sin(5t), 0, 2*pi, t)

;; 3-petal rose 
plotpol(sin(3*t), 0, pi, t)
```
#### Spirals
```
;; Archimedean spiral 
plotpol(t, 0, 4*pi, t)

;; Logarithmic spiral 
plotpol(exp(t/10), 0, 10*pi, t)
```
#### Classic Curves
```
;; Cardioid: r = 1 + cos(θ) 
plotpol(1+cos(t), 0, 2*pi, t)

;; Limaçon: r = 1 + 2cos(θ) 
plotpol(1+2cos(t), 0, 2 pi, t)

;; Circle: r = constant 
plotpol(3, 0, 2*pi, t)
```
---

## Parametric Plots

Plots where both $x$ and $y$ are functions of a parameter: $x = f(t)$, $y = g(t)$.

### Functions

| Function | Description |
|----------|-------------|
| `plotxy(x_expr, y_expr, from, to, var)` | Display parametric plot on screen |
| `fplotxy(filename, x_expr, y_expr, from, to, var)` | Save parametric plot to file |
| `oplotxy(filename, x_expr, y_expr, from, to, var)` | Overlay parametric plot on existing file |

### Syntax
`plotxy(x_expression, y_expression, param_start, param_end, parameter)`

### Features

- Uniform scale (preserves aspect ratio)
- Centered coordinate system
- Cartesian grid
- Suitable for oscilloscope-like displays

### Lissajous Figures

Lissajous figures are created by parametric equations:

$$x = A \sin(at + \delta), \quad y = B \sin(bt)$$
```
;; 3:2 frequency ratio 
plotxy(sin(3t), sin(2t), 0, 2*pi, t)

;; 3:2 with phase shift 
plotxy(sin(3t), cos(2t+1), 0, 2*pi, t)

;; 5:4 ratio 
plotxy(sin(5t), sin(4t), 0, 2*pi, t)
```
### Cycloids
```
;; Cycloid (wheel rolling) 
plotxy(t-sin(t), 1-cos(t), 0, 4*pi, t)

;; Trochoid 
plotxy(t-0.5sin(t), 1-0.5cos(t), 0, 4*pi, t)

;; Epicycloid 
plotxy((5+3)cos(t)-3cos((5+3)*t/3), (5+3)sin(t)-3sin((5+3)t/3), 0, 15 pi, t)
```
### Other Parametric Curves
```
;; Parametric spiral 
plotxy(t cos(t), t sin(t), 0, 10*pi, t)

;; Figure-8 
plotxy(sin(t), sin(2t), 0, 2*pi, t)

;; Lemniscate 
plotxy(cos(t)/(1+sin(t)^2), sin(t)cos(t)/(1+sin(t)^2), 0, 2*pi, t)

;; Butterfly curve 
plotxy(sin(t)(exp(cos(t))-2cos(4t)-sin(t/12)^5), cos(t)(exp(cos(t))-2cos(4t)-sin(t/12)^5), 0, 12*pi, t)
```
---

## Logarithmic Plots

Plots with logarithmic scale on one or both axes. Essential for frequency response analysis.

### Functions

#### Semi-log X (Linear Y, Log X)

| Function | Description |
|----------|-------------|
| `plotlgx(expr, from, to, var)` | Display on screen |
| `fplotlgx(filename, expr, from, to, var)` | Save to file |
| `oplotlgx(filename, expr, from, to, var)` | Overlay on file |

#### Semi-log Y (Log Y, Linear X)

| Function | Description |
|----------|-------------|
| `plotlgy(expr, from, to, var)` | Display on screen |
| `fplotlgy(filename, expr, from, to, var)` | Save to file |
| `oplotlgy(filename, expr, from, to, var)` | Overlay on file |

#### Log-Log (Both axes logarithmic)

| Function | Description |
|----------|-------------|
| `plotlgxy(expr, from, to, var)` | Display on screen |
| `fplotlgxy(filename, expr, from, to, var)` | Save to file |
| `oplotlgxy(filename, expr, from, to, var)` | Overlay on file |

### Syntax
```
plotlgx(expression, start_value, end_value, variable) 
plotlgy(expression, start_value, end_value, variable) 
plotlgxy(expression, start_value, end_value, variable)
```

### Features

- Logarithmic grid with decade markers (1, 10, 100, 1000, ...)
- Minor grid lines (2, 3, 4, 5, 6, 7, 8, 9 within each decade)
- Automatic handling of invalid values (≤0 for log axes)
- Major grid lines marked with thicker lines

### When to Use Each Type

#### Semi-log X (`plotlgx`)
Best for:
- Frequency response (magnitude vs frequency)
- Bode plots (gain in dB vs log frequency)
```
;; Low-pass filter response (dB) 
plotlgx(20 log10(abs(1/(1+i2 pi f/1k))), 10, 100k, f)

;; RC filter magnitude 
plotlgx(abs(1/(1+i2 pi f 50*10n)), 100, 1M, f)
```
#### Semi-log Y (`plotlgy`)
Best for:
- Exponential growth/decay
- Logarithmic scales
```
;; Exponential growth 
plotlgy(exp(t), 0, 10, t)

;; Exponential decay 
plotlgy(exp(-t/5), 0, 20, t)
```

#### Log-Log (`plotlgxy`)
Best for:
- Power law relationships
- Impedance magnitude vs frequency
```
;; Power law: y = x^n 
plotlgxy(x^2, 0.1, 100, x)

;; Impedance magnitude 
plotlgxy(abs(50 + i2*pi f 100n), 1M, 1G, f)
```

### Filter Frequency Response Examples

#### RC Low-Pass Filter

Transfer function: $H(f) = \frac{1}{1 + j2\pi fRC}$
```
;; Magnitude response (normalized) 
plotlgx(abs(1/(1+i2 pi*f/1k)), 10, 100k, f)

;; Magnitude in dB 
plotlgx(20log10(abs(1/(1+i2 pi f/1k))), 10, 100k, f)

;; Phase response 
plotlgx(arg(1/(1+i2 pi*f/1k))*180/pi, 10, 100k, f)
```

#### RL High-Pass Filter
```
;; Magnitude 
plotlgx(abs(i2 pi f 100u/(50+i2 pi f 100u)), 100, 100k, f)
```
#### RLC Resonator
```
;; Parallel RLC 
plotlgxy(abs(1/(1/1k + i2 pi f 10p + 1/(i2 pi f 1u))), 10M, 200M, f)

;; Series RLC 
plotlgxy(abs(10 + i2 pi f 100n + 1/(i2 pi f 2.5p)), 50M, 2G, f)
```

---

## Smith Chart

Specialized plot for RF and microwave engineering. Displays complex impedance on a normalized reflection coefficient plane.

### Functions

#### Default Z0 = 50Ω

| Function | Description |
|----------|-------------|
| `plotsmith(expr, from, to, var)` | Display on screen |
| `fplotsmith(filename, expr, from, to, var)` | Save to file |
| `oplotsmith(filename, expr, from, to, var)` | Overlay on file |

#### Custom Z0

| Function | Description |
|----------|-------------|
| `plotsmithz(expr, from, to, var, Z0)` | Display on screen |
| `fplotsmithz(filename, expr, from, to, var, Z0)` | Save to file |
| `osmithz(filename, expr, from, to, var, Z0)` | Overlay on file |

### Syntax
```
plotsmith(Z_expression, freq_start, freq_end, freq_variable) 
plotsmithz(Z_expression, freq_start, freq_end, freq_variable, reference_impedance)
```

### Parameters

- **expr**: Complex impedance expression $Z = R + jX$
- **from/to**: Frequency range
- **var**: Frequency variable (typically `f`)
- **Z0**: Reference impedance (default: 50Ω)

### Theory

The Smith chart maps complex impedance to reflection coefficient:

$$\Gamma = \frac{Z - Z_0}{Z + Z_0}$$

Where:
- $\Gamma$ is the complex reflection coefficient
- $Z$ is the complex impedance
- $Z_0$ is the reference impedance

The magnitude of $\Gamma$ represents the standing wave ratio (SWR):

$$\text{SWR} = \frac{1 + |\Gamma|}{1 - |\Gamma|}$$

### Smith Chart Features

- Constant resistance circles
- Constant reactance arcs
- Automatic frequency markers at axis crossings
- Real axis (horizontal): purely resistive impedance
- Imaginary axis (vertical): reactive component zero
- Upper half: inductive reactance (+jX)
- Lower half: capacitive reactance (-jX)

### Interpreting the Smith Chart

#### Key Points

| Position | Impedance | Meaning |
|----------|-----------|---------|
| Center | $Z = Z_0$ | Perfect match (SWR = 1:1) |
| Right edge | $Z = \infty$ | Open circuit |
| Left edge | $Z = 0$ | Short circuit |
| Real axis | $X = 0$ | Purely resistive |
| Top of circle | $X = +jZ_0$ | Maximum inductive |
| Bottom of circle | $X = -jZ_0$ | Maximum capacitive |

#### Movement on the Chart

- **Clockwise** motion typically indicates increasing frequency
- **Crossing real axis** indicates resonance (purely resistive)
- **Crossing imaginary axis** indicates transition between capacitive and inductive

### RLC Circuit Examples

#### Series RC

Impedance: $Z = R + \frac{1}{j\omega C}$
```
;; R=50Ω, C=1nF, sweep 1MHz-1GHz 
plotsmith(50 + 1/(i2 pi f 1n), 1M, 1G, f)
```

Characteristics:
- Starts capacitive (lower half)
- Moves toward center as frequency increases
- Crosses real axis at resonance

#### Series RL

Impedance: $Z = R + j\omega L$
```
;; R=50Ω, L=100nH, sweep 1MHz-1GHz 
plotsmith(50 + i2 pi f 100n, 1M, 1G, f)
```

Characteristics:
- Starts near center (low frequency)
- Moves upward (inductive)
- Higher frequencies show increased reactance

#### Series RLC

Impedance: $Z = R + j\omega L + \frac{1}{j\omega C} = R + j\left(\omega L - \frac{1}{\omega C}\right)$
```
;; R=50Ω, L=100nH, C=1pF 
plotsmith(50 + i2 pi f 100n + 1/(i2 pi f 1p), 100M, 10G, f)
```

Resonance frequency: $f_0 = \frac{1}{2\pi\sqrt{LC}} \approx 1.59$ GHz

Characteristics:
- Below $f_0$: capacitive (lower half)
- At $f_0$: crosses real axis (marker shows frequency!)
- Above $f_0$: inductive (upper half)

#### Parallel RLC

Admittance: $Y = \frac{1}{R} + j\omega C + \frac{1}{j\omega L}$

Impedance: $Z = \frac{1}{Y}$
```
;; R=1kΩ, L=1µH, C=10pF 
plotsmith(1/(1/1k + i2 pi f 10p + 1/(i2 pi f 1u)), 10M, 200M, f)
```
Resonance: $f_0 = \frac{1}{2\pi\sqrt{LC}} \approx 50.3$ MHz

Characteristics:
- Maximum impedance at resonance
- Moves through right side (high impedance)
- Real axis crossing shows resonant frequency

### Transmission Line Examples

#### Short Circuit Stub

Input impedance: $Z_{in} = jZ_0 \tan(\beta l)$
```
;; Quarter-wave stub at 1GHz 
plotsmith(i50 tan(2 pi f/1G*pi/2), 500M, 2G, f)
```

#### Open Circuit Stub

Input impedance: $Z_{in} = -jZ_0 \cot(\beta l)$
```
;; Quarter-wave open stub at 1GHz 
plotsmith(-i50/tan(2 pi f/1G pi/2), 500M, 2G, f)
```

### Matching Network Examples

#### L-Match (50Ω → 200Ω)
```
;; Series L, shunt C matching network 
;; L=25nH, C=8pF, matching around 1GHz 
plotsmith(i2 pi f 25n + 200/(1 + i2 pi f 200*8p), 500M, 2G, f)
```
#### Pi-Filter
```
;; Input C, series L, output C 
;; C1=10pF, L=50nH, C2=10pF 
plotsmith(i2 pi f 50n + 1/(i2 pi f 10p), 100M, 5G, f)
```
### Antenna Impedance
```
;; Simple antenna model: R + jX 
;; Radiation resistance + reactive component 
plotsmith(55 + i2 pi f 10u + 1/(i2 pi f 100p), 1M, 30M, f)
```
### Comparing Before/After Matching
```
;; Original antenna impedance 
fplotsmith("antenna.bmp", 75 + i2 pi f 5u, 1M, 30M, f)

;; With matching network 
oplotsmith("antenna.bmp", 75 + i2 pi f 5u + 1/(i2 pi f 200p), 1M, 30M, f)
```
---

## Common Parameters

### Configuration Variables

These variables control plot appearance (set before calling plot functions):

| Variable | Default | Description |
|----------|---------|-------------|
| `plot_width` | 800 | Plot width in pixels (100-2000) |
| `plot_height` | 600 | Plot height in pixels (100-2000) |
| `plot_bgc` | 0xFFFFFF | Background color (RGB hex) |
| `plot_fgc` | 0x000000 | Foreground/trace color (RGB hex) |

### Setting Configuration
plot_width := 1024; plot_height := 768; 
plot_bgc := 0xFFFFFF    ;; white background 
plot_fgc := 0x0000FF    ;; blue trace
plot(sin(x), 0, 2*pi, x)

### Color Format

Colors are specified as 24-bit RGB hex values:
- `0xRRGGBB`
- `0x000000` = Black
- `0xFFFFFF` = White
- `0xFF0000` = Red
- `0x00FF00` = Green
- `0x0000FF` = Blue

---

## Examples

### Comparing Representations

Same circuit, different visualizations:
```
;; Parallel RLC resonator 
;; R=1kΩ, L=1µH, C=10pF, f0≈50.3MHz
;; 1. Impedance magnitude (log-log) 
plotlgxy(abs(1/(1/1k + i2 pi f 10p + 1/(i2 pi f 1u))), 10M, 200M, f)

;; 2. Smith chart (shows impedance trajectory) 
plotsmith(1/(1/1k + i2 pi f 10p + 1/(i2 pi f 1u)), 10M, 200M, f)

;; 3. Phase response 
plotlgx(arg(1/(1/1k + i2 pi f 10p + 1/(i2 pi f 1u)))*180/pi, 10M, 200M, f)
```

### Multi-Trace Overlay
```
;; Create base plot 
fplot("comparison.bmp", sin(x), 0, 2*pi, x)

;; Add more traces 
oplot("comparison.bmp", cos(x), 0, 2*pi, x) 
oplot("comparison.bmp", sin(2x), 0, 2*pi, x)
```
### Complete Filter Analysis
```
;; RC low-pass filter: fc = 1/(2πRC) = 1kHz 
;; R=1kΩ, C=160nF
;; Magnitude response in dB 
fplotlgx("lp_mag.bmp", 20log10(abs(1/(1+i2 pi f 1k 160n))), 10, 100k, f)

;; Phase response 
fplotlgx("lp_phase.bmp", arg(1/(1+i2 pi f 1k*160n))*180/pi, 10, 100k, f)

;; Group delay: τ = -dφ/dω 
fplotlgx("lp_delay.bmp", 1k 160n/(1+(2 pi f 1k*160n)^2), 10, 100k, f)
```
### Complex Parametric Curves
```
;; Spirograph pattern 
plotxy((5+3)cos(t)-3cos((5/3+1)*t), (5+3)sin(t)-3sin((5/3+1)t), 0, 6*pi, t)

;; Hypotrochoid 
plotxy((3-1)cos(t)+1cos((3-1)*t/1), (3-1)sin(t)-1sin((3-1)t/1), 0, 2*pi, t)

---

## Tips and Best Practices

### Frequency Range Selection

For RF circuits:
- **Audio**: 20 Hz - 20 kHz
- **RF/AM**: 100 kHz - 30 MHz
- **VHF/FM**: 30 MHz - 300 MHz
- **UHF**: 300 MHz - 3 GHz
- **Microwave**: 3 GHz - 30 GHz

### Resolution and Smoothness

The calculator automatically generates enough points for smooth curves:
- Cartesian: ~2× plot width
- Polar: ~4× radius
- Parametric: ~2× plot width
- Logarithmic: ~2× plot width
- Smith: ~50× radius (high resolution for complex curves)

### Logarithmic Scale Guidelines

- Always use positive ranges for logarithmic axes
- Span at least 1-2 decades for meaningful visualization
- Use log-log for power relationships
- Use semi-log X for frequency response
- Use semi-log Y for exponential processes

### Smith Chart Guidelines

- Use frequency sweep that shows complete impedance behavior
- Look for real axis crossings (resonances)
- Center of chart = matched load (SWR=1:1)
- Trajectory shows how impedance changes with frequency
- Upper half = inductive, lower half = capacitive

### File Format

All plotting functions save to BMP (bitmap) format:
- 24-bit RGB color
- Uncompressed
- Compatible with all image viewers
- Easy to convert to PNG/JPG if needed

---

## Function Summary Table

| Function Type | Screen | File | Overlay |
|--------------|--------|------|---------|
| **Cartesian** | `plot` | `fplot` | `oplot` |
| **Polar** | `plotpol` | `fplotpol` | `oplotpol` |
| **Parametric** | `plotxy` | `fplotxy` | `oplotxy` |
| **Log X** | `plotlgx` | `fplotlgx` | `oplotlgx` |
| **Log Y** | `plotlgy` | `fplotlgy` | `oplotlgy` |
| **Log XY** | `plotlgxy` | `fplotlgxy` | `oplotlgxy` |
| **Smith (Z₀=50Ω)** | `plotsmith` | `fplotsmith` | `oplotsmith` |
| **Smith (custom Z₀)** | `plotsmithz` | `fplotsmithz` | `osmithz` |

---

## Mathematical Functions Reference

### Complex Number Functions

| Function | Description | Example |
|----------|-------------|---------|
| `abs(z)` | Magnitude: $\|z\| = \sqrt{\text{Re}^2 + \text{Im}^2}$ | `abs(3+4i)` → 5 |
| `arg(z)` or `pol(z)` | Phase angle in radians | `arg(3+4i)` → 0.927 |
| `re(z)` | Real part | `re(3+4i)` → 3 |
| `im(z)` | Imaginary part | `im(3+4i)` → 4 |
| `conj(z)` | Complex conjugate | `conj(3+4i)` → 3-4i |

### Converting to Degrees

Phase is returned in radians. Convert to degrees:
arg(z) * 180/pi

### Decibel Functions

| Function | Description | Formula |
|----------|-------------|---------|
| `db(x)` | Convert to decibels | $20\log_{10}(x)$ |
| `np(x)` | Convert to nepers | $\ln(x)$ |

---

## Version Information

- **Author**: APICalc Development Team
- **License**: See repository
- **Repository**: https://github.com/dimorlus/APIcalc

For bug reports and feature requests, please visit the GitHub repository.