# CASE Study: Automated NTC Thermistor Approximation & Code Generation

### 1. Introduction: The Embedded Engineering Dilemma

Non-Linear Temperature Coefficient (NTC) thermistors are widely used for temperature measurement due to their low cost and sensitivity. However, their temperature-to-resistance characteristic is highly non-linear, governed by exponential relationships (such as the Beta formula or the Steinhart-Hart equation).

Executing floating-point exponential or logarithmic math (using `math.h`) on an 8-bit microcontroller (like PIC or AVR) is extremely resource-intensive, consuming valuable flash memory and hundreds of instruction cycles. In simple words, it is impossible.

The standard engineering solution is **Piecewise Linear Approximation (PWLA)** — dividing the non-linear curve into multiple smaller segments where the temperature can be calculated using fast, integer-only linear interpolation. However, manually calculating lookup tables (`Toffs`) and segment slopes (`Tslop`) in Excel for every change in circuit values or sensor models is tedious and error-prone.

This case study demonstrates how to use the built-in scripting engine and GUI to completely automate this workflow: from a raw hardware schematic definition to a fully optimized, ready-to-compile C code module.

---

### 2. Direct Physical Model of the Divider Circuit

Instead of forcing the engineer to manually derive complex inverse logarithmic equations on paper, the scripting engine allows you to define the circuit's physics exactly as it is built in hardware.

Consider a classic voltage divider where the NTC thermistor is placed in the **lower arm** ($R_L$) and a fixed $6.8\text{ k}\Omega$ resistor is 
placed in the **upper arm** connected to ground. Using a 256-step (8-bit) ADC, we can describe the circuit behavior directly:

```
;; NTC Resistance vs Temperature (Kelvin/Celsius conversion included)
{ntcr(r25, t) B:=3k95; T0:=stdt; T25:=T0+25; r25 * exp(B * (1/(t+T0) - 1/T25))}

;; Ideal ADC Code vs Temperature (NTC in the lower arm, 6k8 in the upper arm)
{ADC(T) rl:=ntcr(100k,T); 256*rl/(rl+6k8)}
```

Because the NTC is in the lower arm, its resistance drops as temperature rises, causing the voltage across it to decrease. 
Thus, the ADC output is an **inverse/falling curve** (e.g., maximum ADC values correspond to low temperatures, and minimum ADC values correspond to high temperatures).

---
![ntc_diags](ntc_diags.png)<br>
---
### 3. The Power of Numerical Inversion: The `solve` Function

To build our lookup table, we need a function that yields **Temperature as a function of the ADC code** ($T(N)$). Since we only have the forward physical equation ($ADC(T)$), we leverage the engine's capability to numerically invert functions on the fly using the built-in `solve()` routine:

```
{Tc(N) solve(adc(t)-N, 50, t)}
```

For every target ADC step ($N$), `solve()` automatically finds the root of the equation by iterating through the forward physical model, bypassing the need for manual algebraic inversion.

---

### 4. Meta-Scripting and Automated Code Generation

The script acts as a compiler tool, processing the non-linear physical curve into an optimized format for 8-bit microcontrollers.

By selecting a segment step size of $N:=8$ (a power of two), we ensure that runtime calculations on the microcontroller can bypass expensive division operations in favor of ultra-fast bitwise shifts.

The generator loop iterates through the defined ADC range (from $80$ [$0^\circ\text{C}$] to $248$ [$130^\circ\text{C}$]), capturing boundaries, pre-scaling the slope multiplier ($\times 64$), and generating fixed-point coefficients.

#### Advanced Scripting Technique: Dynamic Polymorphism of `if()`

A common issue in simple code generators is handling trailing delimiters (the "last comma problem") when outputting static arrays. The script solves this elegantly within a single loop by utilizing the dynamic return types of the `if()` function:

```
prnf("tc.c", "  %d%c", Toffs[ix], if(ix < NN-1, ',', ' '))
```

Unlike standard C/C++ where a ternary operator requires both branches to share a strict, matching data type, the scripting engine's `if()` is dynamically typed. It evaluates and returns a character literal (`,` or a space `' '`) dynamically based on the loop index. This guarantees a perfectly formatted C array declaration with zero trailing syntax errors.

---

### 5. Verification and Compilation

To validate the generated model, the script combines the theoretical continuous curve and the generated piecewise integer steps onto a single graphic viewport:

```
save("lappr.bmp", plot(Tc(n), xmin, xmax, n) + plot(run("tc.txt"), n))
```

While calculating this plot on the PC takes a few seconds due to the nested execution of numerical solver loops for each pixel coordinate, it highlights the stark efficiency contrast: the complex numerical crunching is performed entirely inside the IDE during the design phase, yielding a lightweight, deterministic C production runtime.

The output `tc.c` file is generated fully autonomously, stripping out all high-overhead math dependencies and reducing the microcontroller's runtime footprint to an array lookup, a bit shift, and basic integer subtraction.

---

## Documentation Developer Notes (Add as callout blocks)

> 📌 **Note on Function Inversion (`solve` vs `inv`):**
> For steep, highly non-linear exponential curves (such as NTC thermistor divider circuits), the built-in `solve()` function provides greater numerical stability over broader search intervals than Brent's method (`inv`). It is highly resilient against sharp derivative spikes near the boundaries of the operating window.

> 📌 **Important Syntax Note for C/C++ Programmers:**
> The script's native `if(cond, expr_true, expr_false)` statement acts as a functional expression rather than a structural control flow statement. It can dynamically evaluate and return entirely different data types (e.g., strings, characters, vectors, or scalars) from its respective branches depending on the runtime state. This enables highly compact and expressive formatting patterns during automated text and code generation.

**mcu_ntc.txt:**
```
;; NTC divider piecewise linear approximation generator (Scaled range: ADC 80..248)
;;Start conditions
N:=8
xmin:=80;;~0`C
xmax:=248;;~130`C
;;Precalculations
NN := (248-80)/N
ix := 0
;;Array definitions
Toffs := zeros(NN)
Tslop := zeros(NN)

;;Functions definitions
{ntcr(r25, t) B:=3k95; T0:=stdt; T25:=T0+25; r25 * exp(B * (1/(t+T0) - 1/T25))} ;;ntc(t) from datasheet
{ADC(T) rl:=ntcr(100k,T);256*rl/(rl+6k8)} ;;ADC(T) digitized voltage from divider
{Tc(N) solve(adc(t)-N, 50, t)};;Inverse function calculation

;;Main loop
:loop
 x1 := ix*N+xmin
 x2 := (ix+1)*N+xmin 
 y1 := Tc(x1); y1 := if(y1<0,0,y1)
 y2 := Tc(x2); y2 := if(y2<0,0,y2)
 Toffs[ix] := round(y1) 
 Tslop[ix] := round(-(y2-y1)*64/(x2-x1))
 ix += 1
 NN - ix
JNZ loop

BREAK 

;;save("toffs.txt", Toffs)
;;save("tslop.txt", Tslop)
prnf("","Toffs:=%S", Toffs)
prnf("","Tslop:=%S", Tslop)
;;RET
prnf("tc.txt","");;Create simulation script file
prnf("tc.txt","Toffs:=%S", Toffs)
prnf("tc.txt","Tslop:=%S", Tslop)

prnf("tc.txt","n:=if(n<%d,0,n-%d)", xmin, xmin)
prnf("tc.txt","idx := int(n / %d)",N)
prnf("tc.txt","offset := Toffs[idx]")
prnf("tc.txt","slope := Tslop[idx]")
prnf("tc.txt","T_res := int(offset - int(((n %% %d) * slope) / 64))",N)

BREAK
prnf("tc.c","");;Create C file (for MCU)
prnf("tc.c","uint8_t thermo(uint8_t adc)")
prnf("tc.c","{")
prnf("tc.c"," const uint8_t Toffs = {")
ix := 0
:loffs
 prnfs("tc.c","  %d%c",Toffs[ix],if(ix<NN-1,',',' '))
 ix += 1
 NN - ix
JNZ loffs
prnf("tc.c"," };")
prnf("tc.c"," const uint8_t Tslop = {")
ix := 0
:lslop
 prnfs("tc.c","  %d%c",Tslop[ix],if(ix<NN-1,',',' '))
 ix += 1
 NN - ix
JNZ lslop
prnf("tc.c"," };")
prnf("tc.c"," ")
prnf("tc.c","  uint8_t i, offset, slope;")
prnf("tc.c","  adc=(adc<%d)?0:adc-%d;", xmin, xmin)
prnf("tc.c","  i = adc / %d;", N)
prnf("tc.c","  offset = Toffs[i];")
prnf("tc.c","  slope = Tslop[i];")
prnf("tc.c","  return (uint8_t)(offset - ((uint16_t)(adc %% %d)) * slope / 64);", N)
prnf("tc.c","}")
prnf("tc.c"," ")

BREAK

RET
```

**tc.txt:**
```
Toffs:=[(131, 125, 120, 115, 110, 105, 101); (97, 92, 88, 84, 79, 75, 70); (66, 61, 55, 49, 43, 35, 25)]
Tslop:=[(46, 43, 41, 39, 37, 36, 35); (35, 34, 34, 35, 35, 36, 38); (40, 43, 47, 54, 64, 82, 124)]
n:=if(n<80,0,n-80)
idx := int(n / 8)
offset := Toffs[idx]
slope := Tslop[idx]
T_res := int(offset - int(((n % 8) * slope) / 64))
```

**tc.c (autoformatted):**
```C
uint8_t thermo (uint8_t adc)
{
 const uint8_t Toffs = { 131, 125, 120, 115, 110, 105, 101, 97, 92, 88, 84,
                         79,  75,  70,  66,  61,  55,  49,  43, 35, 25 };
 const uint8_t Tslop = { 46, 43, 41, 39, 37, 36, 35, 35, 34, 34, 35, 35, 
                         36, 38, 40, 43, 47, 54, 64, 82, 124 };

 uint8_t i, offset, slope;
 adc    = (adc < 80) ? 0 : adc - 80;
 i      = adc / 8;
 offset = Toffs[i];
 slope  = Tslop[i];
 return (uint8_t)(offset - ((uint16_t)(adc % 8)) * slope / 64);
}
```