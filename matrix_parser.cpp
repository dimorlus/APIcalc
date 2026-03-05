// Matrix parser for calculator
// Called with pos pointing to char after '['
// Format: [(1, 2, 3);(4, 5, 6);(7, 8, 9)]
// buf and pos are class fields, dscan() advances pos automatically
// Returns pointer to allocated Matrix on success, NULL on error (errorf called)
// On success pos points to char after ']'

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

// Helper: skip spaces at current pos
void calculator::mxSkipSpaces()
{
    while (buf[pos] == ' ' || buf[pos] == '\t') pos++;
}

Matrix *calculator::parseMatrix()
{
    // ----------------------------------------------------------------
    // PASS 1: count rows and columns, check consistency
    // ----------------------------------------------------------------
    int startPos = pos;
    int rows     = 0;
    int cols     = 0;
    int curCols  = 0;

    mxSkipSpaces();

    while (buf[pos])
    {
        if (buf[pos] == ']') break;

        // expect '(' to open a row
        if (buf[pos] != '(')
        {
            errorf(pos, "Expected '(' at start of matrix row");
            return NULL;
        }
        pos++; // skip '('
        rows++;
        if (rows > MAX_R)
        {
            errorf(pos, "Too many rows (max %d)", MAX_R);
            return NULL;
        }

        curCols = 0;
        mxSkipSpaces();

        while (buf[pos] && buf[pos] != ')')
        {
            mxSkipSpaces();

            // nested parens not allowed - no expressions, just numbers
            if (buf[pos] == '(')
            {
                errorf(pos, "Nested parentheses not allowed in matrix");
                return NULL;
            }

            // skip one token: optional sign + number chars until delimiter
            bool hasChars = false;
            if (buf[pos] == '+' || buf[pos] == '-') pos++;
            while (buf[pos] &&
                   buf[pos] != ',' && buf[pos] != ')' &&
                   buf[pos] != ' ' && buf[pos] != '\t')
            {
                if (buf[pos] == '(')
                {
                    errorf(pos, "Nested parentheses not allowed in matrix");
                    return NULL;
                }
                hasChars = true;
                pos++;
            }
            if (!hasChars)
            {
                errorf(pos, "Empty element in matrix row %d", rows);
                return NULL;
            }

            curCols++;
            if (curCols > MAX_C)
            {
                errorf(pos, "Too many columns (max %d)", MAX_C);
                return NULL;
            }

            mxSkipSpaces();
            if (buf[pos] == ',') pos++; // skip ',' between elements
            mxSkipSpaces();
        }

        if (buf[pos] != ')')
        {
            errorf(pos, "Expected ')' at end of matrix row %d", rows);
            return NULL;
        }
        pos++; // skip ')'

        if (rows == 1)
        {
            cols = curCols;
            if (cols == 0)
            {
                errorf(pos, "Empty matrix row");
                return NULL;
            }
        }
        else if (curCols != cols)
        {
            errorf(pos, "Row %d has %d columns, expected %d", rows, curCols, cols);
            return NULL;
        }

        mxSkipSpaces();
        if      (buf[pos] == ';') { pos++; mxSkipSpaces(); } // between rows
        else if (buf[pos] != ']')
        {
            errorf(pos, "Expected ';' or ']' after matrix row %d", rows);
            return NULL;
        }
    }

    if (buf[pos] != ']')
    {
        errorf(pos, "Expected ']' at end of matrix");
        return NULL;
    }
    if (rows == 0 || cols == 0)
    {
        errorf(pos, "Empty matrix");
        return NULL;
    }

    // ----------------------------------------------------------------
    // PASS 2: allocate and fill using dscan()
    // ----------------------------------------------------------------
    Matrix *m = new Matrix();
    if (!m) { errorf(pos, "Out of memory"); return NULL; }

    m->rows = rows;
    m->cols = cols;
    m->data = new float__t[rows * cols];
    if (!m->data) { delete m; errorf(pos, "Out of memory"); return NULL; }

    pos = startPos; // rewind to start of matrix content
    mxSkipSpaces();

    for (int r = 0; r < rows; r++)
    {
        pos++; // skip '('
        mxSkipSpaces();

        for (int c = 0; c < cols; c++)
        {
            t_operator op = dscan();
            if (op != t_number || err[0])
            {
                errorf(pos, "Expected number at [%d][%d]", r, c);
                delete m;
                return NULL;
            }
            m->data[r * cols + c] = result_fval; // dscan stores result here

            mxSkipSpaces();
            if (c < cols - 1)
            {
                if (buf[pos] != ',')
                {
                    errorf(pos, "Expected ',' between elements at row %d", r);
                    delete m;
                    return NULL;
                }
                pos++; // skip ','
                mxSkipSpaces();
            }
        }

        mxSkipSpaces();
        if (buf[pos] != ')')
        {
            errorf(pos, "Expected ')' at end of row %d", r);
            delete m;
            return NULL;
        }
        pos++; // skip ')'
        mxSkipSpaces();

        if (r < rows - 1)
        {
            pos++; // skip ';'
            mxSkipSpaces();
        }
    }

    pos++; // skip ']' — pos now points to char after ']'
    return m;
}


bool negative = false;
if (*ipos == '-') { negative = true; ipos++; }
else if (*ipos == '+') { ipos++; }
while (isspace(*ipos & 0x7f)) ipos++; // на случай "- 3"

pos = ipos - buf + 1;
if (dscan(true, percent) != toOPERAND)
{
    error("Invalid number in matrix");
    free(mval);
    return toERROR;
}
ipos = buf + pos;
tag = v_stack[v_sp - 1].tag;
if (tag == tvINT || tag == tvFLOAT)
{
    fval = v_stack[--v_sp].fval;
    if (negative) fval = -fval;        // применяем знак
    mval[r * cols + c] = fval;
}

if (*ipos == '+' || *ipos == '-') ipos++;
while (isspace(*ipos & 0x7f)) ipos++; // <- добавить
while (*ipos && *ipos != ',' && ...)


