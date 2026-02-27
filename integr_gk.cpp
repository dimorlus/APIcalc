// Gauss-Kronrod G7/K15 adaptive quadrature
// C++98 compatible (BCB6 / VS2022)
//
// K15 nodes (positive half, index 0 = center node = 0.0)
// G7 uses nodes at indices 0, 2, 4, 6  (every other K15 node)

static const float__t GK_NODES[8] = {
    0.0L,
    0.20778495500789846760L,
    0.40584515137739716691L,
    0.58608723546769113029L,
    0.74153118559939443986L,
    0.86486442335976907279L,
    0.94910791234275852453L,
    0.99145537112081263921L,
};

// K15 weights: index i corresponds to nodes +-GK_NODES[i]
// (index 0 has no symmetry — centre point)
static const float__t K15_WEIGHTS[8] = {
    0.20948214108472782801L,
    0.20443294007529889241L,
    0.19035057806478540991L,
    0.16900472663926790283L,
    0.14065325971552591875L,
    0.10479001032224928880L,
    0.06309209262997855329L,
    0.02293532201052922497L,
};

// G7 weights: 4 values for nodes at GK_NODES[0,2,4,6]
static const float__t G7_WEIGHTS[4] = {
    0.41795918367346938776L,
    0.38183005050511894495L,
    0.27970539148927664160L,
    0.12948496616886732340L,
};

// G7 node indices into GK_NODES[]
static const int G7_IDX[4] = { 0, 2, 4, 6 };

// ---------------------------------------------------------------------------

struct GKResult
{
    float__t value;
    float__t error;
    bool     ok;
};

// Evaluate f(x) = sexpr with variable svar set to x in child calculator
// Returns qnan on any error
float__t calculator::gkEval (calculator *pCalc,
                              const char *sexpr,
                              const char *svar,
                              float__t    x)
{
    pCalc->addfvar (svar, x);
    float__t val = pCalc->evaluate (sexpr);
    if (pCalc->err[0])
        return qnan;
    return val;
}

// Single G7/K15 panel on [a, b], no recursion
GKResult calculator::gkPanel (calculator *pCalc,
                               const char *sexpr,
                               const char *svar,
                               float__t    a,
                               float__t    b)
{
    GKResult res = { 0.0L, 0.0L, true };
    float__t center = (a + b) / 2.0L;
    float__t half   = (b - a) / 2.0L;

    // f values at all 15 points: fL[i] = f(center - half*node[i])
    //                            fR[i] = f(center + half*node[i])
    // index 0: fL[0] == fR[0] == f(center)
    float__t fL[8], fR[8];
    fL[0] = fR[0] = gkEval (pCalc, sexpr, svar, center);
    if (isnan (fL[0])) { res.ok = false; return res; }

    for (int i = 1; i < 8; i++)
    {
        fL[i] = gkEval (pCalc, sexpr, svar, center - half * GK_NODES[i]);
        if (isnan (fL[i])) { res.ok = false; return res; }
        fR[i] = gkEval (pCalc, sexpr, svar, center + half * GK_NODES[i]);
        if (isnan (fR[i])) { res.ok = false; return res; }
    }

    // K15: sum over all 8 node pairs (index 0 counted once)
    float__t k15 = K15_WEIGHTS[0] * fL[0];
    for (int i = 1; i < 8; i++)
        k15 += K15_WEIGHTS[i] * (fL[i] + fR[i]);
    k15 *= half;

    // G7: sum over node indices 0, 2, 4, 6
    float__t g7 = G7_WEIGHTS[0] * fL[0];
    for (int i = 1; i < 4; i++)
    {
        int idx = G7_IDX[i];
        g7 += G7_WEIGHTS[i] * (fL[idx] + fR[idx]);
    }
    g7 *= half;

    res.value = k15;
    res.error = fabsl (k15 - g7);
    return res;
}

// Adaptive G7/K15: recursively subdivide until error < tol or maxDepth reached
GKResult calculator::gkAdaptive (calculator *pCalc,
                                  const char *sexpr,
                                  const char *svar,
                                  float__t    a,
                                  float__t    b,
                                  float__t    tol,
                                  int         depth,
                                  int         maxDepth)
{
    GKResult res = gkPanel (pCalc, sexpr, svar, a, b);
    if (!res.ok)
        return res;

    if (res.error <= tol || depth >= maxDepth)
        return res;

    float__t mid = (a + b) / 2.0L;
    float__t halfTol = tol / 2.0L;

    GKResult left  = gkAdaptive (pCalc, sexpr, svar, a,   mid, halfTol, depth + 1, maxDepth);
    GKResult right = gkAdaptive (pCalc, sexpr, svar, mid, b,   halfTol, depth + 1, maxDepth);

    if (!left.ok || !right.ok)
    {
        GKResult bad = { qnan, qnan, false };
        return bad;
    }

    GKResult combined;
    combined.value = left.value + right.value;
    combined.error = left.error + right.error;
    combined.ok    = true;
    return combined;
}

// ---------------------------------------------------------------------------
// Main Integr() — вставить перед последним delete pCalculator
// ---------------------------------------------------------------------------
//
//   GKResult gkresult = gkAdaptive (pCalculator,
//                                   sexpr,        // выражение
//                                   svar,         // имя переменной ("x")
//                                   vfrom,        // нижний предел
//                                   vto,          // верхний предел
//                                   1e-10L,       // tolerance
//                                   0,            // начальная глубина
//                                   20);          // максимальная глубина
//   if (!gkresult.ok)
//    {
//     errorf (pos, "Integration failed");
//     result_fval = qnan;
//     delete pCalculator;
//     return qnan;
//    }
//   result = gkresult.value;
