## Data Plot Functions

Plot data points or lines from text files with flexible data extraction using masks.

### Syntax
```
plotdata(datafile [, mask])           ;; Display points in window
plotdatal(datafile [, mask])          ;; Display lines in window
fplotdata(bmpfile, datafile [, mask]) ;; Save points to BMP file
fplotdatal(bmpfile, datafile [, mask]);; Save lines to BMP file
oplotdata(bmpfile, datafile [, mask]) ;; Overlay points on existing BMP
oplotdatal(bmpfile, datafile [, mask]);; Overlay lines on existing BMP
```
### Parameters

- **datafile** - Path to text file containing numerical data
- **mask** - Optional string pattern for data extraction (default: first number in each line)
- **bmpfile** - Output bitmap file path (for `f*` and `o*` variants)

### Mask Format

The mask string controls which numbers to capture and in what order:

- **Numbers (0-9)**: Index of argument to capture, starting from 0
  - `0` captures first argument (X coordinate)
  - `1` captures second argument (Y coordinate)
- **Wildcards (`*`, `-`, `#`, etc.)**: Any non-numeric, non-space character skips one number
- **Spaces**: Optional visual separators (ignored during parsing)

**Mask behavior:**
- **No mask**: X = line number, Y = first number in line
- **1 digit (e.g., `"0"`)**: X = line number, Y = number at specified position
- **2 digits (e.g., `"01"`)**: X = first captured value, Y = second captured value
- **More than 2 digits**: Error

### Examples

#### Basic Usage

**Data file** (`test.txt`):
```
10 20 30
15 25 35
20 30 40

plotdata("test.txt")        ;; No mask: X = line number, Y = Val0
                            ;; Points: (1,10), (2,15), (3,20)

plotdata("test.txt", "0")   ;; X = line number, Y = Val0
                            ;; Same as above: (1,10), (2,15), (3,20)

plotdata("test.txt", "*0")  ;; X = line number, Y = Val1 (skip first, take second)
                            ;; Points: (1,20), (2,25), (3,30)

plotdata("test.txt", "01")  ;; X = Val0, Y = Val1
                            ;; Points: (10,20), (15,25), (20,30)

plotdatal("test.txt", "*01");; X = Val1, Y = Val2 (skip first, take 2nd & 3rd)
                            ;; Lines: (20,30), (25,35), (30,40)

plotdata("test.txt", "10")  ;; X = Val1, Y = Val0 (reversed)
                            ;; Points: (20,10), (25,15), (30,20)
```

#### Complex Format Parsing

**Comma-separated data** (`sensors.csv`):
```
2024-01-15, 10:30, 23.5, 45.2, OK
2024-01-15, 11:00, 24.1, 44.8, OK
2024-01-15, 11:30, 25.0, 43.9, OK
```
Parsed as: `[1705314600, 23.5, 45.2, ...]` (Unix time, seconds from 1970-01-01 00:00:00)

```
plotdata("sensors.csv", "*0")  ;; X = line number, Y = Val5 (temperature)
                                   ;; Skip 1 value, capture 23.5, 24.1, 25.0

plotdata("sensors.csv", "*01") ;; X = Val5 (temp), Y = Val6 (humidity)
                                   ;; Points: (23.5,45.2), (24.1,44.8), (25.0,43.9)
```
**Mixed delimiters** (`log.txt`):
```
Time: 100ms Voltage: 3.3V Current: 0.5A
Time: 200ms Voltage: 3.4V Current: 0.6A
Time: 300ms Voltage: 3.2V Current: 0.4A
```
Parsed as: `[100, 3.3, 0.5]` per line (keywords ignored)
```
plotdata("log.txt", "01")  ;; X = Val0 (time), Y = Val1 (voltage)
                           ;; Points: (100,3.3), (200,3.4), (300,3.2)

plotdata("log.txt", "*01") ;; X = Val1 (voltage), Y = Val2 (current)
                           ;; Skip first number (time)
                           ;; Points: (3.3,0.5), (3.4,0.6), (3.2,0.4)
```
#### Real-World Example: NTC Thermistor
```
Create thermistor resistance table:
{ntcr(r25, t) B:=3950; T0:=273.15; T25:=T0+25; r25 * exp(B * (1/(t+T0) - 1/T25))}
for(prnf("ntc.txt","%d`C, %S", temp, ntcr(100k, temp)), -40, 120, temp)
```
File `ntc.txt`:
```
-40`C, 4.019M
-39`C, 3.738M
...
125`C, 3.588k
```
Visualize data:
```
plotdatal("ntc.txt", "01")  ;; Temperature → Resistance curve
                            ;; X = Val0 (temp), Y = Val1 (resistance)

plotdatal("ntc.txt", "10")  ;; Resistance → Temperature (inverse)
                            ;; X = Val1 (resistance), Y = Val0 (temp)

fplotdatal("ntc_curve.bmp", "ntc.txt", "01")  ;; Save to file
```

**Note:** Engineering format output uses SI prefixes (k, M, G, m, u, n) without special Unicode characters. The `%S` format specifier automatically selects appropriate prefix.

#### Date Handling

**Engineering log with timestamps**:
```
2026-04-22 10:00:05 102.5 0.985
2026-04-22 10:05:10 104.2 0.990
2026-04-22 10:10:15 103.8 0.987
```
Parsed as: `[1776848405, 102.5, 0.985]`

```
plotdata("log.txt", "*01") ;; Skip entire timestamp 
                           ;; X = Val6 (value), Y = Val7 (quality)
                           ;; Points: (102.5,0.985), (104.2,0.990), ...

plotdata("log.txt", "* 0")  ;; Skip timestamp, plot value over line number
                            ;; X = line number, Y = Val6
```
#### Saving and Overlaying

```
;; Save base plot
fplotdatal("comparison.bmp", "data1.txt", "01")

;; Overlay additional data with different extraction
oplotdatal("comparison.bmp", "data2.txt", "*01")  ;; Skip first column
oplotdata("comparison.bmp", "data3.txt", "0")     ;; Add points, single column
```

### Features

- **Automatic scaling**: Axes automatically adjust to data range
- **Grid and labels**: Standard Cartesian grid with value labels
- **Zero inclusion**: Y-axis always includes zero for better reference
- **Flexible parsing**: Handles various text formats (CSV, space-delimited, mixed)
- **Engineering notation**: Supports suffixes (k, M, G, m, u, n, etc.)
- **Scientific notation**: Parses exponential format (1.5e-3, 2.3E6)
- **Robust parsing**: Invalid lines (incomplete data, wrong format) are automatically skipped

### Plot Settings

Control appearance using global variables:
```
plot_width := 1024     ;; Canvas width in pixels (default: 800)
plot_height := 768     ;; Canvas height in pixels (default: 600)
plot_bgc := 0xFFFFFF   ;; Background color (default: white)
plot_fgc := 0x0000FF   ;; Foreground color (default: black)
plot_dotsz := 3        ;; Point size for plotdata (default: 4)
```

#### Advanced Color Settings

Use temperature-to-RGB or wavelength-to-RGB conversion for natural color plots:

**Black body radiation colors:**
```
plot_bgc := 0
plot_fgc := trgb(3000)  ;; Warm white (3000 Kelvin)
plotdatal("data.txt", "01")  ;; CT = 3000K
```
**Visible spectrum colors:**
```
plot_bgc := 0
plot_fgc := wrgb(orange)    ;; ~600nm (orange wavelength)
plotdatal("data.txt", "01")

plot_fgc := wrgb(532n)      ;; 532nm (green laser)
plot_fgc := wrgb(red)       ;; ~685nm (red wavelength)
plot_fgc := wrgb(blue)      ;; ~472nm (blue wavelength)
```

Available spectrum colors: `red`, `orange`, `yellow`, `green`, `blue`, `indigo`, `violet`

### Notes

- Empty lines and lines without valid numbers are skipped
- Lines with insufficient data (mask requires more numbers than available) are ignored
- File path supports environment variables: `%TEMP%\data.txt`
- Relative paths resolved from `path` variable if set
- Maximum 10 numbers per line can be indexed (0-9)
- Non-numeric text (units, labels) is automatically ignored during parsing

### Error Messages

- `"Cannot open data file"` - File not found or access denied
- `"No valid data points found"` - File contains no parseable numbers matching mask
- `"Mask has more than 2 parameters"` - Mask contains more than 2 digit characters
- `"Cannot reopen data file"` - File became unavailable between passes

### See Also

- [plot() - Function plotting](plot.md#function-plots)
- [trgb(), wrgb() - Color temperature and wavelength conversion](functions.md#color)
- [fitpoly(), fitexp() - Regression with masks](regression.md)
- [mean(), median() - Statistics with masks](statistics.md)
- [prnf() - Formatted file output](functions.md#prnf)