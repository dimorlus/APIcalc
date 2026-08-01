# WAV Audio Support in Calculator

This calculator includes comprehensive support for WAV audio generation, manipulation, and analysis.

## Table of Contents

- [Generating WAV Files](#generating-wav-files)
- [WAV Operations](#wav-operations)
- [File Operations](#file-operations)
- [Reading WAV Values](#reading-wav-values)
- [FFT Analysis](#fft-analysis)
- [Harmonic Synthesis](#harmonic-synthesis)
- [Examples](#examples)

---

## Generating WAV Files

### `play(expression, from, to, variable)`

Generates a WAV audio file from a mathematical expression.

**Parameters:**
- `expression` - Mathematical expression to evaluate (signal amplitude)
- `from` - Start time in seconds
- `to` - End time in seconds
- `variable` - Time variable name

**Returns:** WAV object (mono, 44100 Hz, 16-bit PCM)

**Example:**

    # Generate 440 Hz sine wave (A4 note) for 1 second
    f := 440
    w := play(sin(t*2*pi*f), 0, 1, t)
    
    # Generate chord (multiple frequencies)
    w := play(sin(t*2*pi*440) + sin(t*2*pi*554.37)/2 + sin(t*2*pi*659.25)/3, 0, 2, t)

**Notes:**
- The expression is normalized to the maximum amplitude found during generation
- Two-pass algorithm: first pass finds maximum, second generates samples
- Duration is specified in seconds: `to - from`

---

## WAV Operations

WAV objects support arithmetic operations similar to numbers.

### Addition: `wav1 + wav2` or `wav + scalar`

**WAV + WAV:** Mixes two audio signals

    w1 := play(sin(t*2*pi*440), 0, 1, t)
    w2 := play(sin(t*2*pi*880), 0, 1, t)
    mixed := w1 + w2  # Mix both signals

**WAV + scalar:** Adds DC offset (values in range [-1.0, 1.0])

    w := play(sin(t*2*pi*440), 0, 1, t)
    shifted := w + 0.5  # Shift signal up by 50% of maximum amplitude

### Subtraction: `wav1 - wav2` or `wav - scalar`

**WAV - WAV:** Subtracts one signal from another

    result := w1 - w2  # Signal difference

**WAV - scalar:** Removes DC offset

    centered := w - 0.1  # Shift signal down

### Multiplication: `wav1 * wav2` or `wav * scalar`

**WAV * WAV:** Element-wise multiplication (modulation)

    carrier := play(sin(t*2*pi*1000), 0, 1, t)
    modulator := play(sin(t*2*pi*10), 0, 1, t)
    am_signal := carrier * modulator  # Amplitude modulation

**WAV * scalar:** Volume control

    quiet := w * 0.5  # Reduce volume by 50%
    loud := w * 2.0   # Double volume (may clip)

### Division: `wav / scalar`

Divides amplitude by scalar (volume reduction)

    w := play(sin(t*2*pi*440), 0, 1, t)
    quiet := w / 2  # Reduce volume by half

### Concatenation: `wav1 | wav2`

Joins two WAV files sequentially (plays first, then second)

    w1 := play(sin(t*2*pi*440), 0, 0.5, t)
    w2 := play(sin(t*2*pi*880), 0, 0.5, t)
    sequence := w1 | w2  # Play w1 for 0.5s, then w2 for 0.5s (total 1s)

**Requirements:** Both WAV files must have same sample rate, channels, and bit depth.

---

## File Operations

### `save(filename, wav)`

Saves a WAV object to a file.

**Example:**

    w := play(sin(t*2*pi*440), 0, 1, t)
    save("tone_440hz.wav", w)

### `load(filename)`

Loads a WAV file from disk.

**Example:**

    w := load("tone_440hz.wav")
    # Now can perform operations
    louder := w * 1.5
    save("tone_440hz_loud.wav", louder)

**Note:** `load()` automatically detects file type (WAV, BMP, or text).

---

## Reading WAV Values

### `wav(wavObject, t)`

Reads a normalized sample value from a WAV object at a specific time.

**Parameters:**
- `wavObject` - WAV object to read from
- `t` - Time in seconds

**Returns:** Normalized amplitude value in range [-1.0, 1.0], or 0 if time is out of bounds

**Example:**

    # Create WAV file
    f := 440
    w := play(sin(t*2*pi*f), 0, 1, t)
    
    # Read value at specific time
    y := wav(w, 0.5)  # Value at t=0.5 seconds
    
    # Plot the entire WAV content
    plot(wav(w, t), 0, 1, t)
    
    # Zoom into first 10 milliseconds
    plot(wav(w, t), 0, 0.01, t)
    
    # Reading beyond file duration returns 0
    y := wav(w, 5.0)  # Returns 0 (file is only 1 second)

**Features:**
- Linear interpolation between samples for smooth values
- Returns 0 for time values outside the WAV duration
- Can be used in mathematical expressions and plotting

**Use cases:**

    # Use WAV as modulation source
    carrier := 1000
    modulated := play(sin(t*2*pi*carrier) * wav(w, t), 0, 1, t)
    
    # Combine multiple WAV files with timing
    w1 := play(sin(t*2*pi*440), 0, 0.5, t)
    w2 := play(sin(t*2*pi*880), 0, 0.5, t)
    combined := play(wav(w1, t) + wav(w2, t-0.25), 0, 0.75, t)
    
    # Reverse playback (read from end to start)
    duration := 1.0
    reversed := play(wav(w, duration-t), 0, duration, t)

---

## FFT Analysis

### `fft(wav)`

Performs Fast Fourier Transform on a WAV file to extract dominant frequencies.

**Parameters:**
- `wav` - WAV object to analyze

**Returns:** Matrix with 2 columns [frequency, amplitude] and up to 7 rows (top 7 harmonics)

**Example:**

    # Create signal with known frequencies
    f := 440
    w := play(sin(t*2*pi*f) + sin(t*2*pi*f*2)/2, 0, 1, t)
    
    # Analyze
    h := fft(w)
    
    # Result matrix (approximately):
    # [(440.1,  764.7m);
    #  (880.2,  375.1m)]

**Algorithm:**
- Uses Cooley-Tukey FFT algorithm
- FFT size is power of 2 (max 32768 samples)
- Finds local peaks in magnitude spectrum
- Returns top 7 harmonics sorted by amplitude

**Note:** Small frequency and amplitude deviations are normal due to:
- Frequency bin quantization (frequency resolution = sampleRate / fftSize)
- Spectral leakage (signal may not contain integer number of periods)
- Numerical precision

---

## Harmonic Synthesis

### `ifft(harmonics, duration)`

Synthesizes a WAV file from a harmonics matrix.

**Parameters:**
- `harmonics` - Matrix with 2 columns [frequency, amplitude]
- `duration` - Duration in seconds

**Returns:** WAV object

**Example:**

    # Define harmonics manually
    h := [(440, 1.0); (880, 0.5); (1320, 0.25)]
    
    # Synthesize 2-second WAV
    w := ifft(h, 2.0)
    
    # Save
    save("synthesized.wav", w)

### `harmonic(harmonics, t)`

Evaluates harmonic sum at a specific time point.

**Parameters:**
- `harmonics` - Matrix with 2 columns [frequency, amplitude]
- `t` - Time in seconds

**Returns:** Signal amplitude at time t

**Example:**

    h := [(440, 1.0); (880, 0.5)]
    
    # Evaluate at specific time
    y := harmonic(h, 0.005)  # Value at t=5ms
    
    # Plot the signal
    plot(harmonic(h, t), 0, 0.01, t)  # Plot first 10ms

**Use case:** Visualize synthesized signal without creating full WAV file.

---

## Examples

### Example 1: Musical Notes

    # Define note frequencies (A4, C#5, E5 - A major chord)
    a4 := 440
    cs5 := 554.37
    e5 := 659.25
    
    # Generate chord
    chord := play(sin(t*2*pi*a4) + sin(t*2*pi*cs5) + sin(t*2*pi*e5), 0, 2, t)
    
    # Save
    save("a_major_chord.wav", chord)

### Example 2: Frequency Sweep

    # Linear sweep from 440Hz to 880Hz over 2 seconds
    f := 440 + (880-440)*t/2
    sweep := play(sin(t*2*pi*f), 0, 2, t)
    save("sweep_440_880.wav", sweep)

### Example 3: Beat Frequency

    # Two close frequencies create beating effect
    f1 := 440
    f2 := 445
    beats := play(sin(t*2*pi*f1) + sin(t*2*pi*f2), 0, 4, t)
    save("beats_5hz.wav", beats)

### Example 4: Analysis and Resynthesis

    # Create original signal
    original := play(sin(t*2*pi*440) + sin(t*2*pi*880)/2, 0, 1, t)
    
    # Analyze
    harmonics := fft(original)
    
    # Resynthesize
    reconstructed := ifft(harmonics, 1.0)
    
    # Compare by plotting
    plot(harmonic(harmonics, t), 0, 0.01, t)

### Example 5: Audio Processing Chain

    # Generate tone
    w := play(sin(t*2*pi*440), 0, 1, t)
    
    # Reduce volume
    w := w * 0.7
    
    # Add DC offset (shift up)
    w := w + 0.1
    
    # Create sequence: original, pause, processed
    pause := play(0, 0, 0.5, t)
    result := w | pause | w
    
    # Save
    save("processed_sequence.wav", result)

### Example 6: Amplitude Modulation

    # Carrier frequency: 1000 Hz
    # Modulation frequency: 10 Hz
    carrier := play(sin(t*2*pi*1000), 0, 2, t)
    modulator := play(0.5 + 0.5*sin(t*2*pi*10), 0, 2, t)
    am_signal := carrier * modulator
    save("am_modulation.wav", am_signal)

### Example 7: Visualizing WAV Content

    # Generate complex signal
    f := 440
    w := play(sin(t*2*pi*f) + sin(t*4*pi*f)/2, 0, 1, t)
    
    # Plot entire waveform
    plot(wav(w, t), 0, 1, t)
    
    # Zoom into first 10 milliseconds
    plot(wav(w, t), 0, 0.01, t)
    
    # Compare original expression with stored WAV
    plot(sin(t*2*pi*f) + sin(t*4*pi*f)/2, 0, 0.01, t)
    plot(wav(w, t), 0, 0.01, t)

### Example 8: WAV-based Modulation

    # Create modulation envelope
    envelope := play(0.5 + 0.5*sin(t*2*pi*2), 0, 2, t)
    
    # Use envelope to modulate a tone
    tone := play(sin(t*2*pi*440) * wav(envelope, t), 0, 2, t)
    save("envelope_modulated.wav", tone)

---

## Technical Details

### WAV Format Specifications
- **Format:** PCM (uncompressed)
- **Sample Rate:** 44100 Hz
- **Bit Depth:** 16-bit signed integer
- **Channels:** 1 (mono)
- **Endianness:** Little-endian (Windows standard)

### WAV Header Structure

    RIFF chunk (12 bytes)
    fmt chunk (24 bytes)
    data chunk (8 bytes + samples)

### Memory Management
- WAV objects are allocated with `malloc()`
- Automatically freed when variable is reassigned or goes out of scope
- Use `save()` to persist WAV data to disk

### Performance Considerations
- FFT limited to 32768 samples for performance
- First pass in `play()` uses 22050 Hz sampling for speed
- Second pass generates full 44100 Hz output
- Large WAV files (> 10 seconds) may take time to process
- `wav()` function uses linear interpolation for smooth reading

### Limitations
- FFT returns maximum 7 harmonics (matrix row limit)
- WAV operations require compatible formats (same sample rate, channels, bit depth)
- Amplitude clipping occurs at ±1.0 normalized range

---

## Variable Display

WAV variables are displayed in the format:

    wavvar = WAV(duration, sampleRate, channels)

Example:

    w := play(sin(t*2*pi*440), 0, 2.5, t)
    # Displays: w = WAV(2.500s,44100Hz,1ch)

---

## Future Enhancements

Planned features:
- `plotfft(wav)` - Visual FFT spectrum plot
- `harmonic(mx, z)` - Complex argument support
- Stereo WAV support (2 channels)
- Additional window functions for FFT
- Convolution and filtering operations