### Statistical Analysis
#### Overview
The statistics family of functions provides a comprehensive toolkit for analyzing datasets directly from external files. 
From basic counters to robust measures of central tendency, these functions are optimized for real-world engineering data, 
where noise and outliers are common.
#### Key Features
* Memory Efficiency: Most functions use single-pass algorithms (like Welford’s method for variance). 
The median function uses an iterative binary search, allowing it to process multi-gigabyte logs with near-zero RAM usage.
* Smart Filtering: Like the regression engine, the statistics module automatically filters out text headers, timestamps, 
and comments, focusing only on the numeric values.

#### Function Reference

* **num("filename")**: Returns the total count of valid numeric entries found in the file.<br>
Example: ``num("test.log")`` — useful for verifying data integrity.
* **mean("filename")**: Calculates the arithmetic mean ($$\mu$$) of all values.<br>
Returns: A single scalar value representing the DC component or average level.
* **median("filename")**: Provides a robust estimate of the central value.<br>
*Note*: Unlike the mean, the median is resistant to "glitches" and extreme outliers. 
It is calculated using a high-precision iterative binary search on the value range.
* **rms("filename")**: Calculates the Root Mean Square (effective value).<br>
* **sumx("filename")**: Returns the plain sum of all values in the dataset.
* **stddevp("filename")**: Calculates the Population Standard Deviation ($$\sigma$$). 
Use this when the file contains the entire set of data you are interested in.
* **stddevs("filename")**: Calculates the Sample Standard Deviation ($$s$$).
Use this when the file is a sample of a larger process. (Uses Bessel's correction: divisor is$$n-1$$).
* **min("filename") / max("filename")**: Find the absolute minimum and maximum values.<br>
Returns: The peak values, useful for determining the dynamic range or signal clipping.