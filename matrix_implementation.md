# Matrix Implementation Summary

## Syntax
```
[(1, 2, 3);(4, 5, 6);(7, 8, 9)]
```
- Rows separated by `;`
- Elements in a row separated by `,`
- Each row wrapped in `()`
- Whole matrix wrapped in `[]`
- Spaces allowed anywhere
- Nested parentheses NOT allowed — no expressions, only numbers
- Negative numbers: `-3.14` — minus handled by parseMatrix, not dscan

## Struct

```cpp
#define MAX_R 64
#define MAX_C 64

struct Matrix
{
    float__t *data;   // row-major: data[r*cols + c]
    int       rows;
    int       cols;

    Matrix() : data(NULL), rows(0), cols(0) {}
   ~Matrix() { delete[] data; }
};
```

## Class members to add (in calculator.h)

```cpp
// Matrix support
Matrix *parseMatrix();           // main parser, pos points to char after '['
void    mxSkipSpaces();          // skip spaces at current pos
```

## parseMatrix() logic

### Pass 1 — count rows/cols, check consistency
- Scan structure only, no number parsing
- Check `rows <= MAX_R`, `cols <= MAX_C`
- Check all rows have same number of columns
- Check no nested `(`
- On any error: call `errorf()`, return `NULL`

### Pass 2 — allocate and fill
- `new Matrix()`, `new float__t[rows * cols]`
- Rewind `pos = startPos`
- For each element:
  - Skip spaces
  - Check and consume optional leading `+` or `-` sign manually
  - Call `dscan()`
  - If `dscan()` returned `toOPERAND`: pop `v_stack[--v_sp].fval`
  - Apply sign if negative
  - Store in `m->data[r * cols + c]`
- On any error: `delete m`, call `errorf()`, return `NULL`
- On success: `pos++` to skip `]`, return `m`

## Calling parseMatrix()

In the main parser, when `[` is encountered:
```cpp
pos++; // skip '['

// disable Fahrenheit conversion inside matrix — not meaningful there
unsigned saved_scfg = scfg;
scfg &= ~FRH;

Matrix *m = parseMatrix();

scfg = saved_scfg; // restore regardless of success/failure

if (!m)
{
    // errorf already called inside parseMatrix
    result_fval = qnan;
    return toERROR;
}

// push matrix onto operand stack as new type
// v_stack[v_sp].matrix = m;
// v_stack[v_sp].type   = tMATRIX;
// v_sp++;
// return toOPERAND;
```

## Operand stack — adding matrix type

The `v_stack` entry needs to accommodate a matrix pointer.
Simplest approach — add to existing union:

```cpp
union t_operand
{
    float__t  fval;
    char     *sval;    // existing string type
    Matrix   *mval;    // new matrix type
};

// and a type tag if not already present:
enum t_optype { tFLOAT, tSTRING, tMATRIX };

struct t_stack_entry
{
    t_operand value;
    t_optype  type;
};
```

**Important**: when popping a matrix from the stack and discarding it
(e.g. on error), call `delete` to free memory:
```cpp
if (v_stack[v_sp].type == tMATRIX)
    delete v_stack[v_sp].value.mval;
```

## What's NOT implemented yet (future)
- Matrix arithmetic: `+`, `-`, `*` (element-wise and true matrix multiply)
- Scalar × matrix
- Transpose: `tr(M)` or `M'`
- Determinant: `det(M)`
- Inverse: `inv(M)`
- `solve` with matrix (linear systems)
- Output formatting for matrix results
- Named matrix variables: `A := [(1,2);(3,4)]`
