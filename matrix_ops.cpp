// Matrix operations for calculator
// matrixbin: binary operations, matrixuno: unary operations

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

// Allocate a new matrix rows*cols, register for auto-free
// Returns NULL on failure (error already set)
float__t *calculator::mxAlloc (int rows, int cols)
{
 float__t *mval = (float__t *)malloc (rows * cols * sizeof (float__t));
 if (!mval)
  {
   error ("Memory allocation failed for matrix");
   return NULL;
  }
 registerString ((char *)mval);
 return mval;
}

// Fill res from left op right, element-wise, both must be same-size matrices
// op: 0=add, 1=sub, 2=mul, 3=div
bool calculator::mxElemOp (value &res, value &left, value &right, int op)
{
 if (left.mrows != right.mrows || left.mcols != right.mcols)
  {
   error ("Matrix dimensions must match");
   return false;
  }
 int rows       = left.mrows;
 int cols       = left.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 switch (op)
  {
  case 0: for (int i = 0; i < n; i++) mval[i] = left.mval[i] + right.mval[i]; break;
  case 1: for (int i = 0; i < n; i++) mval[i] = left.mval[i] - right.mval[i]; break;
  case 2: for (int i = 0; i < n; i++) mval[i] = left.mval[i] * right.mval[i]; break;
  case 3: for (int i = 0; i < n; i++) mval[i] = left.mval[i] / right.mval[i]; break;
  }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// Fill res from matrix op scalar or scalar op matrix
// side: 0 = matrix op scalar, 1 = scalar op matrix
bool calculator::mxScalarOp (value &res, value &mx, float__t scalar, int op, bool scalar_left)
{
 int rows       = mx.mrows;
 int cols       = mx.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 switch (op)
  {
  case 0: // add: scalar+M == M+scalar
   for (int i = 0; i < n; i++) mval[i] = mx.mval[i] + scalar;
   break;
  case 1: // sub: M-scalar vs scalar-M
   if (scalar_left)
    for (int i = 0; i < n; i++) mval[i] = scalar - mx.mval[i];
   else
    for (int i = 0; i < n; i++) mval[i] = mx.mval[i] - scalar;
   break;
  case 2: // mul: scalar*M == M*scalar
   for (int i = 0; i < n; i++) mval[i] = mx.mval[i] * scalar;
   break;
  case 3: // div: M/scalar vs scalar/M (element-wise)
   if (scalar_left)
    for (int i = 0; i < n; i++) mval[i] = scalar / mx.mval[i];
   else
    for (int i = 0; i < n; i++) mval[i] = mx.mval[i] / scalar;
   break;
  }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// True matrix multiplication: res = left * right
bool calculator::mxMatMul (value &res, value &left, value &right)
{
 if (left.mcols != right.mrows)
  {
   error ("Matrix dimensions incompatible for multiplication");
   return false;
  }
 int rows       = left.mrows;
 int cols       = right.mcols;
 int inner      = left.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 for (int r = 0; r < rows; r++)
  for (int c = 0; c < cols; c++)
   {
    float__t sum = 0.0L;
    for (int k = 0; k < inner; k++)
     sum += left.mval[r * inner + k] * right.mval[k * cols + c];
    mval[r * cols + c] = sum;
   }
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// ---------------------------------------------------------------------------
// matrixbin — binary operations
// ---------------------------------------------------------------------------
t_mresult calculator::matrixbin (value &res, value &left, value &right, t_operator cop)
{
 if (left.tag != tvMATRIX && right.tag != tvMATRIX)
  return mrSKIP; // not a matrix operation

 bool lm = (left.tag  == tvMATRIX);
 bool rm = (right.tag == tvMATRIX);
 bool ls = left.is_scalar();
 bool rs = right.is_scalar();

 switch (cop)
  {
  // ---- ADD ----
  case toADD:
  case toSETADD:
   if (lm && rm)  { if (!mxElemOp   (res, left, right, 0))        return mrERROR; }
   else if (lm)   { if (!mxScalarOp (res, left,  right.fval, 0, false)) return mrERROR; }
   else           { if (!mxScalarOp (res, right, left.fval,  0, true))  return mrERROR; }
   return mrDONE;

  // ---- SUB ----
  case toSUB:
  case toSETSUB:
   if (lm && rm)  { if (!mxElemOp   (res, left, right, 1))              return mrERROR; }
   else if (lm)   { if (!mxScalarOp (res, left,  right.fval, 1, false)) return mrERROR; }
   else           { if (!mxScalarOp (res, right, left.fval,  1, true))  return mrERROR; }
   return mrDONE;

  // ---- MUL ----
  case toMUL:
  case toSETMUL:
   if (lm && rm)  { if (!mxMatMul   (res, left, right))                 return mrERROR; }
   else if (lm)   { if (!mxScalarOp (res, left,  right.fval, 2, false)) return mrERROR; }
   else           { if (!mxScalarOp (res, right, left.fval,  2, true))  return mrERROR; }
   return mrDONE;

  // ---- DIV ----
  case toDIV:
  case toSETDIV:
   if (lm && rm)
    {
     error ("Matrix/matrix division is not defined (use inv())");
     return mrERROR;
    }
   else if (lm)   { if (!mxScalarOp (res, left,  right.fval, 3, false)) return mrERROR; }
   else           { if (!mxScalarOp (res, right, left.fval,  3, true))  return mrERROR; }
   return mrDONE;

  // ---- POW ----
  case toPOW:
  case toSETPOW:
   // M^n — repeated multiplication, only integer n, square matrix
   if (lm && rs)
    {
     if (left.mrows != left.mcols)
      {
       error ("Matrix power requires a square matrix");
       return mrERROR;
      }
     long long n = (long long)right.fval;
     if ((float__t)n != right.fval || n < 0)
      {
       error ("Matrix power requires a non-negative integer exponent");
       return mrERROR;
      }
     // start with identity matrix
     int sz         = left.mrows;
     float__t *mval = mxAlloc (sz, sz);
     if (!mval) return mrERROR;
     // identity
     for (int r = 0; r < sz; r++)
      for (int c = 0; c < sz; c++)
       mval[r * sz + c] = (r == c) ? 1.0L : 0.0L;
     res.tag   = tvMATRIX;
     res.mrows = sz;
     res.mcols = sz;
     res.mval  = mval;
     // multiply n times
     value cur = left;
     for (long long i = 0; i < n; i++)
      {
       value tmp = res;
       if (!mxMatMul (res, tmp, cur)) return mrERROR;
      }
     return mrDONE;
    }
   error ("Matrix power: left must be matrix, right must be non-negative integer scalar");
   return mrERROR;

  // ---- parallel resistors M//M or M//scalar ----
  case toPAR:
   if (lm && rm)
    {
     // element-wise: (a*b)/(a+b)
     if (left.mrows != right.mrows || left.mcols != right.mcols)
      {
       error ("Matrix dimensions must match for // operator");
       return mrERROR;
      }
     int rows       = left.mrows;
     int cols       = left.mcols;
     float__t *mval = mxAlloc (rows, cols);
     if (!mval) return mrERROR;
     for (int i = 0; i < rows * cols; i++)
      mval[i] = (left.mval[i] * right.mval[i]) / (left.mval[i] + right.mval[i]);
     res.tag   = tvMATRIX;
     res.mrows = rows;
     res.mcols = cols;
     res.mval  = mval;
     return mrDONE;
    }
   else if (lm && rs)
    {
     int rows       = left.mrows;
     int cols       = left.mcols;
     float__t *mval = mxAlloc (rows, cols);
     if (!mval) return mrERROR;
     for (int i = 0; i < rows * cols; i++)
      mval[i] = (left.mval[i] * right.fval) / (left.mval[i] + right.fval);
     res.tag   = tvMATRIX;
     res.mrows = rows;
     res.mcols = cols;
     res.mval  = mval;
     return mrDONE;
    }
   else if (ls && rm)
    {
     int rows       = right.mrows;
     int cols       = right.mcols;
     float__t *mval = mxAlloc (rows, cols);
     if (!mval) return mrERROR;
     for (int i = 0; i < rows * cols; i++)
      mval[i] = (left.fval * right.mval[i]) / (left.fval + right.mval[i]);
     res.tag   = tvMATRIX;
     res.mrows = rows;
     res.mcols = cols;
     res.mval  = mval;
     return mrDONE;
    }
   return mrSKIP;

  // ---- comparison, bitwise, shifts — not defined for matrices ----
  case toEQ: case toNE: case toGT: case toGE: case toLT: case toLE:
  case toAND: case toOR: case toXOR:
  case toASL: case toASR: case toLSR:
  case toSETOR: case toSETXOR: case toSETASL: case toSETASR: case toSETLSR:
   error ("Operation not defined for matrices");
   return mrERROR;

  default:
   break;
  }
 return mrSKIP;
}

// ---------------------------------------------------------------------------
// matrixuno — unary operations
// ---------------------------------------------------------------------------
t_mresult calculator::matrixuno (value &res, value &operand, t_operator cop)
{
 if (operand.tag != tvMATRIX)
  return mrSKIP;

 int rows = operand.mrows;
 int cols = operand.mcols;
 int n    = rows * cols;

 switch (cop)
  {
  // ---- unary minus ----
  case toMINUS:
   {
    float__t *mval = mxAlloc (rows, cols);
    if (!mval) return mrERROR;
    for (int i = 0; i < n; i++) mval[i] = -operand.mval[i];
    res.tag   = tvMATRIX;
    res.mrows = rows;
    res.mcols = cols;
    res.mval  = mval;
    return mrDONE;
   }

  // ---- unary plus — no-op ----
  case toPLUS:
   res = operand;
   return mrDONE;

  // ---- ~ transpose (for real matrix = conjugate transpose) ----
  case toCOM:
   {
    float__t *mval = mxAlloc (cols, rows); // note: transposed dimensions
    if (!mval) return mrERROR;
    for (int r = 0; r < rows; r++)
     for (int c = 0; c < cols; c++)
      mval[c * rows + r] = operand.mval[r * cols + c];
    res.tag   = tvMATRIX;
    res.mrows = cols; // transposed
    res.mcols = rows;
    res.mval  = mval;
    return mrDONE;
   }

  // ---- ! logical not — not defined ----
  case toNOT:
   error ("Logical NOT not defined for matrices");
   return mrERROR;

  default:
   break;
  }
 return mrSKIP;
}
