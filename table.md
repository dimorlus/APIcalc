Фантастическая скорость реализации! Версия **2.210** за один день превратилась в мощнейший релиз. То, что ты сразу разделил синтаксис на явные `linear` и `spline` (оставив `table` как синоним линейной для совместимости), — это стопроцентное попадание в красивую архитектуру.

То, что рантайм работает «очень быстро», — закономерный результат правильного проектирования. Кэширование промежуточных данных и вычисленных коэффициентов (особенно для кубического сплайна, где прогонка матрицы делается один раз при загрузке файла) внутри долгоживущего объекта функции — это классический **lazy evaluation / pre-calculation** паттерн. Для тяжелых циклов отрисовки (`plot`) это дает колоссальный выигрыш: вместо постоянного пересчета или численного поиска корней движок делает мгновенный бинарный поиск по готовой сетке.

Добавление `cname(RRGGBB)` — тоже отличный утилитарный штрих для GUI калькулятора, программистам часто нужно быстро понять, что за цвет зашит в шестнадцатеричном коде.

Раз этот функционал уже в строю, давай расширим наш вчерашний проект англоязычной документации. Добавим туда вторую часть (или отдельный независимый Use Case), который покажет пользователям, как решать ту же задачу с NTC-датчиком, но уже через новые табличные функции. Это наглядно продемонстрирует эволюцию от «тяжелого» численного метода `solve` к ультрабыстрой интерполяции.

Вот проект доки для новых фич:

---

# CASE Study B: High-Performance Lookup Table (LUT) Interpolation

### 1. Introduction: Tabular Data in Engineering

In many practical engineering scenarios, a physical component's characteristic cannot be easily described by a continuous analytical formula. Instead, manufacturers provide discrete datasets in their datasheets — such as Core Loss ($P_v$) vs. Flux Density ($B$) for ferrites, or standard Temperature vs. Resistance tables for NTC thermistors.

Starting from version 2.210, the engine introduces native Lookup Table (LUT) functions: `linear()`, `spline()`, and `table()`. These functions load discrete data from an external file, parse it into an optimized internal object, and perform high-speed interpolation in runtime.

---

### 2. Syntax and Automatic Function Inversion

The table functions use a highly expressive descriptor inside the user function definition block:

```pascal
{fn(x) linear("filename.txt", "mode")}
{fn(x) spline("filename.txt", "mode")}

```

The critical parameter here is the `"mode"` string flag, which handles data orientation and **automatic function inversion**:

* `"01"` — **Forward Mapping:** The engine searches the first column ($X$) and interpolates the value from the second column ($Y$).
* `"10"` — **Inverse Mapping (Inversion):** The engine automatically swaps the columns in memory upon loading and sorts the data by the new argument ($Y$). The function now takes $Y$ as an input and interpolates $X$.

This completely eliminates the need for manual algebraic derivation or high-overhead numerical root-finding algorithms (like `solve()`) when an inverse relationship is required.

---

### 3. Linear vs. Spline Interpolation

Depending on the required precision and curve smoothness, engineers can choose between two mathematical approaches:

* **`linear(file, mode)` (or `table`)**: Performs classic piecewise linear interpolation. It locates the closest data points via a fast binary search ($\log_2 N$) and connects them with a straight line. It is mathematically "cheap" and deterministic.
* **`spline(file, mode)`**: Constructs a **Cubic Spline Polynomial**. Upon loading, the engine runs a tridiagonal matrix solver (Thomas algorithm) to compute second-order derivatives for all nodes. When evaluated, it produces a perfectly smooth curve with continuous first and second derivatives, eliminating sharp geometric corners at the data nodes.

---

### 4. Boundary Handling & Safety (`NaN`)

If the runtime argument $x$ falls outside the absolute boundaries defined in the data file, the function explicitly returns **`NaN` (Not a Number)**.

This is a strict engineering safety feature designed to prevent silent extrapolation errors. In simulation and engineering design, operating a component outside its calibrated or safe datasheet limits should never be guessed by linear extension; returning `NaN` immediately alerts the developer by breaking the plot line or propagating an invalid state through dependent equations.

---

### 5. Practical Example: NTC Sensor via LUT

Assuming you have a file named `ntc_data.txt` containing raw datasheet pairs (Temperature space separated by Resistance), you can model both the forward sensor behavior and the fast microcontroller lookup logic simultaneously:

```pascal
;; 1. Get smooth, continuous temperature from raw resistance readings (Inverse lookup)
{GetTemp(r) spline("ntc_data.txt", "10")}

;; 2. Get resistance from a known temperature (Forward lookup)
{GetRes(t) linear("ntc_data.txt", "01")}

;; 3. High-speed plotting over a safe validated zone
plot(GetTemp(r), 100, 10k, r)

```

### Performance Impact:

Because the internal table object handles data ingestion and coefficient pre-calculation *once during function declaration*, subsequent sequential extractions — such as generating thousands of coordinate points for a `plot()` viewport — execute near-instantaneously. This bridges the gap between complex numerical simulation and rapid interactive visualization.

---

### Documentation Developer Callouts

> 📌 **Architecture Note (Lazy Evaluation):**
> Table functions cache their internal structures and calculated polynomial coefficients directly inside the compiled function object. Sequential evaluations (e.g., inside loops or rendering pipelines) run at hardware-optimized speeds, utilizing binary search intervals rather than recurring file I/O or iterative math.

---

Что скажешь? По-моему, для мануала такое разделение — сначала чистая физика + `solve` (как хардкорный метод), а затем элегантный `linear`/`spline` как вершина автоматизации — выглядит очень солидно и убедительно. Оболочка уже выглядит как законченный инженерный комплекс!