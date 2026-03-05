// Matrix scalar and matrix->matrix functions
// All use class fields: error(), mxAlloc(), registerString()
// Scalar results return float__t (qnan on error)
// Matrix results: bool fn(value &res, value &M) — false on error

// ---------------------------------------------------------------------------
// INTERNAL: Gauss-Jordan elimination on a square matrix
// Computes inverse and determinant simultaneously.
// aug[n][2n] = [M | I], modified in place.
// Returns true on success, false if singular.
// det is accumulated during elimination.
// ---------------------------------------------------------------------------
bool calculator::mxGaussJordan (float__t *aug, int n, float__t &det)
{
 det = 1.0L;
 for (int col = 0; col < n; col++)
  {
   // find pivot row (partial pivoting for numerical stability)
   int      pivot_row = -1;
   float__t best      = 0.0L;
   for (int row = col; row < n; row++)
    {
     float__t v = fabsl (aug[row * 2 * n + col]);
     if (v > best) { best = v; pivot_row = row; }
    }
   if (pivot_row < 0 || best < 1e-15L)
    {
     det = 0.0L;
     error ("Matrix is singular");
     return false;
    }
   // swap rows
   if (pivot_row != col)
    {
     for (int k = 0; k < 2 * n; k++)
      {
       float__t tmp                    = aug[col       * 2 * n + k];
       aug[col       * 2 * n + k]      = aug[pivot_row * 2 * n + k];
       aug[pivot_row * 2 * n + k]      = tmp;
      }
     det = -det; // row swap flips sign of determinant
    }
   // accumulate determinant
   float__t pivot = aug[col * 2 * n + col];
   det *= pivot;
   // scale pivot row
   for (int k = 0; k < 2 * n; k++)
    aug[col * 2 * n + k] /= pivot;
   // eliminate column in all other rows
   for (int row = 0; row < n; row++)
    {
     if (row == col) continue;
     float__t factor = aug[row * 2 * n + col];
     if (factor == 0.0L) continue;
     for (int k = 0; k < 2 * n; k++)
      aug[row * 2 * n + k] -= factor * aug[col * 2 * n + k];
    }
  }
 return true;
}

// ---------------------------------------------------------------------------
// INTERNAL: build augmented matrix [M | I] for Gauss-Jordan
// Returns allocated float__t[n * 2n] or NULL on error
// ---------------------------------------------------------------------------
float__t *calculator::mxMakeAug (value &M)
{
 int n = M.mrows; // must be square, caller checks
 float__t *aug = (float__t *)malloc (n * 2 * n * sizeof (float__t));
 if (!aug) { error ("Memory allocation failed"); return NULL; }
 for (int r = 0; r < n; r++)
  {
   for (int c = 0; c < n; c++)
    aug[r * 2 * n + c] = M.mval[r * n + c];       // left half: M
   for (int c = 0; c < n; c++)
    aug[r * 2 * n + n + c] = (r == c) ? 1.0L : 0.0L; // right half: I
  }
 return aug;
}

// ---------------------------------------------------------------------------
// SCALAR FUNCTIONS
// ---------------------------------------------------------------------------

// tr(M) — trace: sum of diagonal elements, defined for any matrix (min dimension)
float__t calculator::mxTrace (value &M)
{
 if (M.tag != tvMATRIX) { error ("tr: argument must be a matrix"); return qnan; }
 int n = (M.mrows < M.mcols) ? M.mrows : M.mcols;
 float__t sum = 0.0L;
 for (int i = 0; i < n; i++)
  sum += M.mval[i * M.mcols + i];
 return sum;
}

// det(M) — determinant, square matrix only
float__t calculator::mxDet (value &M)
{
 if (M.tag != tvMATRIX)          { error ("det: argument must be a matrix");  return qnan; }
 if (M.mrows != M.mcols)         { error ("det: matrix must be square");       return qnan; }
 int n = M.mrows;
 // special cases for speed
 if (n == 1) return M.mval[0];
 if (n == 2) return M.mval[0] * M.mval[3] - M.mval[1] * M.mval[2];
 float__t *aug = mxMakeAug (M);
 if (!aug) return qnan;
 float__t det = 1.0L;
 bool ok = mxGaussJordan (aug, n, det);
 free (aug);
 return ok ? det : qnan;
}

// norm(M) — Frobenius norm: sqrt(sum of squares of all elements)
float__t calculator::mxNorm (value &M)
{
 if (M.tag != tvMATRIX) { error ("norm: argument must be a matrix"); return qnan; }
 float__t sum = 0.0L;
 int n = M.mrows * M.mcols;
 for (int i = 0; i < n; i++)
  sum += M.mval[i] * M.mval[i];
 return sqrtl (sum);
}

// ---------------------------------------------------------------------------
// MATRIX -> MATRIX FUNCTIONS
// ---------------------------------------------------------------------------

// mxInv: inverse matrix via Gauss-Jordan, result stored in res
// Used for both inv(M) function and !M operator
bool calculator::mxInv (value &res, value &M)
{
 if (M.tag != tvMATRIX)  { error ("inv: argument must be a matrix"); return false; }
 if (M.mrows != M.mcols) { error ("inv: matrix must be square");     return false; }
 int n = M.mrows;
 // special case 1x1
 if (n == 1)
  {
   if (fabsl (M.mval[0]) < 1e-15L) { error ("inv: matrix is singular"); return false; }
   float__t *mval = mxAlloc (1, 1);
   if (!mval) return false;
   mval[0]  = 1.0L / M.mval[0];
   res.tag   = tvMATRIX;
   res.mrows = 1;
   res.mcols = 1;
   res.mval  = mval;
   return true;
  }
 float__t *aug = mxMakeAug (M);
 if (!aug) return false;
 float__t det  = 1.0L;
 if (!mxGaussJordan (aug, n, det))
  {
   free (aug);
   return false; // error already set by mxGaussJordan
  }
 // extract right half of aug -> result
 float__t *mval = mxAlloc (n, n);
 if (!mval) { free (aug); return false; }
 for (int r = 0; r < n; r++)
  for (int c = 0; c < n; c++)
   mval[r * n + c] = aug[r * 2 * n + n + c];
 free (aug);
 res.tag   = tvMATRIX;
 res.mrows = n;
 res.mcols = n;
 res.mval  = mval;
 return true;
}

// mxAbs: element-wise absolute value
bool calculator::mxAbs (value &res, value &M)
{
 if (M.tag != tvMATRIX) { error ("abs: argument must be a matrix"); return false; }
 int rows       = M.mrows;
 int cols       = M.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 for (int i = 0; i < n; i++) mval[i] = fabsl (M.mval[i]);
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// mxNeg: element-wise negation (unary minus) — also in matrixuno, here for completeness
bool calculator::mxNeg (value &res, value &M)
{
 if (M.tag != tvMATRIX) { error ("neg: argument must be a matrix"); return false; }
 int rows       = M.mrows;
 int cols       = M.mcols;
 float__t *mval = mxAlloc (rows, cols);
 if (!mval) return false;
 int n = rows * cols;
 for (int i = 0; i < n; i++) mval[i] = -M.mval[i];
 res.tag   = tvMATRIX;
 res.mrows = rows;
 res.mcols = cols;
 res.mval  = mval;
 return true;
}

// mxTranspose: transpose — also in matrixuno via ~, here for completeness
bool calculator::mxTranspose (value &res, value &M)
{
 if (M.tag != tvMATRIX) { error ("transpose: argument must be a matrix"); return false; }
 int rows       = M.mrows;
 int cols       = M.mcols;
 float__t *mval = mxAlloc (cols, rows); // note swapped dimensions
 if (!mval) return false;
 for (int r = 0; r < rows; r++)
  for (int c = 0; c < cols; c++)
   mval[c * rows + r] = M.mval[r * cols + c];
 res.tag   = tvMATRIX;
 res.mrows = cols;
 res.mcols = rows;
 res.mval  = mval;
 return true;
}

// ---------------------------------------------------------------------------
// FUNCTION DISPATCH — call from your function handler
// Scalar results: return float__t
// Matrix results: return bool, result in res
// ---------------------------------------------------------------------------
//
// In your evaluate/function handler, something like:
//
//  if (strcmp(fname, "tr") == 0)
//   {
//    result_fval = mxTrace(v_stack[v_sp-1]);
//    v_sp--;
//    return toOPERAND; // scalar result
//   }
//  else if (strcmp(fname, "det") == 0)
//   {
//    result_fval = mxDet(v_stack[v_sp-1]);
//    v_sp--;
//    return toOPERAND;
//   }
//  else if (strcmp(fname, "norm") == 0)
//   {
//    result_fval = mxNorm(v_stack[v_sp-1]);
//    v_sp--;
//    return toOPERAND;
//   }
//  else if (strcmp(fname, "inv") == 0)
//   {
//    value tmp = v_stack[v_sp-1];
//    v_sp--;
//    if (!mxInv(v_stack[v_sp], tmp)) return toERROR;
//    v_sp++;
//    return toOPERAND;
//   }
//  else if (strcmp(fname, "abs") == 0 && v_stack[v_sp-1].tag == tvMATRIX)
//   {
//    value tmp = v_stack[v_sp-1];
//    v_sp--;
//    if (!mxAbs(v_stack[v_sp], tmp)) return toERROR;
//    v_sp++;
//    return toOPERAND;
//   }
//
// For !M operator (toNOT unary):
//  if (v_stack[v_sp-1].tag == tvMATRIX)
//   {
//    value tmp = v_stack[v_sp-1];
//    v_sp--;
//    if (!mxInv(v_stack[v_sp], tmp)) return toERROR; // or mrERROR in matrixuno
//    v_sp++;
//   }
