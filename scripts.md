# Script Syntax

## Overview

The script engine provides a simple assembly-like language with labels, jumps, calls, and expression evaluation. Scripts are compiled into bytecode for efficient execution.

## Syntax Structure

Lines can contain:
- Comments: lines starting with `;` or `;;`
- Labels: lines starting with `:` followed by a label name (up to 8 characters)
- Operators: JMP, JZ, JNZ, CALL, CALLZ, CALLNZ, RET
- Expressions: any valid calculator expression

Multiple statements can be placed on one line separated by `;`

## Labels

Labels are defined by placing `:` at the start of a line followed by the label name:
```
:labelname
```
Label names:
- Up to 8 characters (longer names are truncated)
- Case-insensitive
- Only alphanumeric characters allowed
- Everything after the label name on the same line is ignored

## Operators

### JMP label
Unconditional jump to the specified label.
```
:target
a := 10
JMP target  ; Jump to 'target'
```

### JZ label
Jump to label if the last expression result equals zero.
```
a := 5
JZ skip     ; Jump if a == 0 (false in this case)
b := 10
:skip
```

### JNZ label
Jump to label if the last expression result is NOT zero.
```
a := 5
JNZ process ; Jump if a != 0 (true in this case)
:process
b := a * 2
```

### CALL label
Call a subroutine at the specified label. Return address is pushed onto the stack.
```

CALL subroutine
; execution continues here after RET

:subroutine
; subroutine code
RET
```

### CALLZ label
Call subroutine only if the last expression result equals zero.
```
a := 0
CALLZ zeroproc  ; Calls because a == 0
:zeroproc
RET
```

### CALLNZ label
Call subroutine only if the last expression result is NOT zero.
```
a := 5
CALLNZ nonzeroproc  ; Calls because a != 0
:nonzeroproc
RET
```

### RET
Return from subroutine. If called from the main script level (stack is empty), exits the script.

## Expression Evaluation

Any line that is not a comment, label, or operator is evaluated as an expression using the calculator engine.
```
a := 10
b := a + 5
c := sqrt(b)
result := if(c > 3, c * 2, c / 2)
```
The result of the last evaluated expression determines the behavior of conditional jump/call operators (JZ, JNZ, CALLZ, CALLNZ).

## Zero Condition

For conditional operators, zero is determined as follows:
- Integer: value == 0
- Float: value == 0.0
- Complex: both real and imaginary parts == 0.0
- Other types: treated as non-zero

## Execution Flow

1. **Load**: Script file is loaded into memory
2. **Parse**: Two-pass parsing
   - First pass: count lines and labels
   - Second pass: build line index and label table
3. **Compile**: Third pass converts operators and labels into bytecode
   - Operators → opcodes (1-7)
   - Labels → line numbers (2 bytes)
   - Label table is freed after compilation
4. **Execute**: Bytecode is interpreted
   - Expression lines are evaluated by the calculator
   - Results are stored and used for conditional operations

## Example Script
```
;; Factorial calculation script
JMP start

:factorial
; Input: n (number to calculate factorial)
; Output: result (factorial of n)
result := 1
:loop
if(n <= 1, 0, 1)
JZ done
result := result * n
n := n - 1
JMP loop
:done
RET

:start
n := 5
CALL factorial
; result now contains 120 (5!)
result

RET
```

## Complete Example

```
;; test script
JMP start

:sub1
a := a + 1
RET

:start
a := 0
:loop
CALL sub1
if(a > 10, 1, 0)
JZ loop
; a is now 11
a
RET ;; exit from script
```

## Limitations

- Maximum 32767 lines per script (uint16_t addressing)
- Maximum 256 nested subroutine calls (stack depth)
- Label names limited to 8 characters
- Only 7 built-in operators

## Implementation Details

### Bytecode Format

Compiled operators are stored as:
- 1 byte: opcode (1-7)
- 2 bytes: target line number (for jump/call operators)
- 1 byte: null terminator

### Operator Opcodes

1. JMP
2. JZ
3. JNZ
4. CALL
5. CALLZ
6. CALLNZ
7. RET

### Return Values

Scripts return the result of the last evaluated expression to the parent calculator. All data types are supported:
- Integers
- Floating-point numbers
- Complex numbers
- Strings
- Matrices

The script returns the result of the last expression evaluated before RET or before reaching the end of the script. This result is passed back to the parent calculator that invoked run("script.txt").

All calculator data types are supported as return values: integers, floating-point numbers, complex numbers, strings, and matrices.

To display or return a specific variable at the end, simply place it as the last expression:
```
a := 42
b := a * 2
a  ; This value will be returned
RET
```
Example with complex number:
```
z := 3 + 4i
magnitude := abs(z)
z  ; Returns 3+4i to parent calculator
RET
```
Example with matrix:
```
m := [[1, 2], [3, 4]]
det := det(m)
m  ; Returns the matrix to parent calculator
RET
```

## Debug Mode (CLI only)

When debug callback is set, execution produces detailed output:

[0000] JMP 7
[0007] EVAL: a := 0
       Result: 0 (int)
[0008] CALL 1 (sp=0)
[0001] EVAL: a := a + 1
       Result: 1 (int)
[0002] RET (sp=1)
       Returned to line 9

Debug output includes:
- Line numbers in [0000] format
- Operator names and arguments
- Expression evaluation results
- Stack depth for calls/returns
- Condition evaluation (true/false)

