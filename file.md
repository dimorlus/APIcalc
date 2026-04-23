### Data Format & File Handling

#### File Requirements
* **Plain Text Only**: Data must be in a standard text format (e.g., .txt, .csv, .log, .dat). Binary formats like 
.xlsx, .pdf, or .docx are not supported.<br>
* **Encoding**: Standard ASCII or UTF-8 is recommended.

#### File Paths & Syntax
* **Flexible Separators**: The engine supports both Windows-style backslashes (`\`) and Unix-style forward slashes (`/`). 
You can even mix them — the parser will correctly locate the file.
* **Flexible Quotes**: Use either double quotes ("path") or single quotes ('path') to define filenames. 
The only requirement is that they must be paired.
* **Universal Paths**: Any combination of forward slashes (`/`) and backslashes (`\`) is valid. The engine transparently 
handles path normalization.

* *Valid*: 'C:/Data/log.txt'
* *Valid*: "logs\sensor_data.csv"
* *Valid*: 'c:\projects/test/data.dat'

#### File Paths:
* *Relative paths* (e.g., "logs/data.txt") are relative to the calculator executable. 
* *Full paths* (e.g., "C:\Projects\Test\sensor.log") are supported.  

#### The "All-Terrain" Parser
The engine uses a robust, fault-tolerant scanner designed to extract numeric data from real-world engineering logs.
* **Header & Comment Stripping**: The parser automatically skips non-numeric text at the beginning of lines, making 
it easy to process files with headers or metadata.Engineering Notation: Full support for standard suffixes: k (kilo, $$10^3$$), 
M (mega, $$10^6$$), etc.
* **Delimiter Agnostic**: While comma-separated values (CSV) are standard, the parser handles spaces, tabs, and semicolons 
gracefully.
