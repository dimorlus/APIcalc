
##Regression & Data Fitting

###Overview

The fit family of functions provides high-precision curve fitting using the Ordinary Least Squares (OLS) method. 
Designed for engineers and hardware developers, these functions can process data directly from text files 
(logs, CSVs, sensor dumps) and return a vector of coefficients for the best-fit model. 

###Key Features
* Stream Processing: Files are read line-by-line, allowing for the analysis of large datasets without high memory overhead.
* Robust Parsing: Uses an "all-terrain" scanner that ignores non-numeric headers and supports engineering notation 
(e.g., 100k, 5m, 2.5u).
* High Precision: All internal calculations are performed using 128-bit floating-point (__float128) math to ensure stability even with high-degree polynomials.Function Reference1. fitpoly("filename", degree)Fits a polynomial of the specified degree to the data pairs $(x, y)$.Degree: $1$ to $6$.Returns: A row vector $[a_n, \dots, a_1, a_0]$ representing the polynomial:$$y = a_n x^n + \dots + a_1 x + a_0$$Example: fitpoly("ntc.txt", 2) returns coefficients for a quadratic curve.2. fitexp("filename")Fits an exponential model: $y = a \cdot e^{bx}$.Returns: $[b, a]$.Note: Automatically performs linearization via $\ln(y)$.3. fitlg("filename")Fits a logarithmic model: $y = a + b \cdot \ln(x)$.Returns: $[b, a]$.4. fitpwr("filename")Fits a power-law model: $y = a \cdot x^b$.Returns: $[b, a]$.Note: Excellent for modeling physical laws and sensor characteristics.5. fitinv("filename")Fits an inverse model: $y = a + b/x$.Returns: $[b, a]$.