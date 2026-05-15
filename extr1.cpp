// Find extremum using Golden Section Search (derivative-free)
// More robust for flat extrema like x^(1/x)
// extr(expr(x), initial, x) -> extr(x^(1/x), 2, x)
float__t calculator::Extremum(const char *expr)
{
    if (!expr || !*expr)
    {
        errorf(pos, "empty expression");
        return result_fval = qnan;
    }

    char sexpr[STRBUF], svar[STRBUF], sinit[MAXOP];
    
    if (!Split(expr, sexpr, STRBUF, sinit, MAXOP, svar, STRBUF, NULL, 0))
    {
        errorf(pos, "extremum: invalid expression");
        return result_fval = qnan;
    }

    calculator *child = new calculator(scfg|SNAN, hash_table, 
                                       (MASK_DEFAULT | MASK_VARIABLE), deep);
    if (!child)
    {
        errorf(pos, "Out of memory");
        return result_fval = qnan;
    }

    float__t x0 = child->evaluate_f(sinit);
    if (isnan(x0) || child->err[0])
    {
        errorf(pos, "%s", child->err);
        delete child;
        return result_fval = qnan;
    }

#ifdef _float128_
    const float__t tol = 1e-32Q;
    const float__t phi_inv = 0.6180339887498948482045868343656Q; // 1/phi
#else
    const float__t tol = 1e-15L;
    const float__t phi_inv = 0.6180339887498948482045868343656L; // 1/phi
#endif

    // Helper: evaluate function at x
    float__t fval = 0;
    bool eval_ok = true;
    
    // Macro-like inline evaluation
    #define EVAL_F(xval, result) \
        do { \
            child->addfvar(svar, xval); \
            result = child->evaluate_f(sexpr); \
            if (isnan(result) && child->errt() != teMath) { \
                if (child->err[0]) { eval_ok = false; break; } \
            } \
            if (isnan(result)) result = 0; \
        } while(0)

    // Step 1: Determine if looking for min or max by checking nearby points
    float__t f0, f_left, f_right;
    float__t probe_step = fabsl(x0) * (float__t)0.1;
    if (probe_step < (float__t)0.1) probe_step = (float__t)0.1;
    
    EVAL_F(x0, f0);
    if (!eval_ok) {
        errorf(pos, "%s", child->err);
        delete child;
        return result_fval = qnan;
    }
    
    EVAL_F(x0 - probe_step, f_left);
    if (!eval_ok) {
        errorf(pos, "%s", child->err);
        delete child;
        return result_fval = qnan;
    }
    
    EVAL_F(x0 + probe_step, f_right);
    if (!eval_ok) {
        errorf(pos, "%s", child->err);
        delete child;
        return result_fval = qnan;
    }
    
    bool seeking_max = (f0 > f_left && f0 > f_right);
    
    // Step 2: Bracket the extremum using derivative sign changes
    float__t xa = x0 - probe_step;
    float__t xb = x0 + probe_step;
    
    // Expand bracket
    int i;
    for (i = 0; i < 20; i++)
    {
        EVAL_F(xa, f_left);
        EVAL_F(xb, f_right);
        EVAL_F((xa + xb) * (float__t)0.5, fval);
        
        if (seeking_max)
        {
            // For max: want f(center) > f(left) and f(center) > f(right)
            if (fval > f_left && fval > f_right)
                break;
        }
        else
        {
            // For min: want f(center) < f(left) and f(center) < f(right)
            if (fval < f_left && fval < f_right)
                break;
        }
        
        // Expand in direction of better value
        if ((seeking_max && f_left > f_right) || (!seeking_max && f_left < f_right))
        {
            xb = (xa + xb) * (float__t)0.5;
            xa = xa - probe_step;
        }
        else
        {
            xa = (xa + xb) * (float__t)0.5;
            xb = xb + probe_step;
        }
        
        probe_step *= (float__t)1.3;
    }
    
    // Step 3: Golden Section Search on [xa, xb]
    float__t a = xa;
    float__t b = xb;
    float__t c = b - (b - a) * phi_inv;
    float__t d = a + (b - a) * phi_inv;
    
    float__t fc, fd;
    EVAL_F(c, fc);
    if (!eval_ok) {
        errorf(pos, "%s", child->err);
        delete child;
        return result_fval = qnan;
    }
    
    EVAL_F(d, fd);
    if (!eval_ok) {
        errorf(pos, "%s", child->err);
        delete child;
        return result_fval = qnan;
    }
    
    for (i = 0; i < 200; i++)
    {
        if (fabsl(b - a) < tol * (fabsl(a) + fabsl(b)))
            break;
        
        bool condition = seeking_max ? (fc > fd) : (fc < fd);
        
        if (condition)
        {
            b = d;
            d = c;
            fd = fc;
            c = b - (b - a) * phi_inv;
            EVAL_F(c, fc);
            if (!eval_ok) {
                errorf(pos, "%s", child->err);
                delete child;
                return result_fval = qnan;
            }
        }
        else
        {
            a = c;
            c = d;
            fc = fd;
            d = a + (b - a) * phi_inv;
            EVAL_F(d, fd);
            if (!eval_ok) {
                errorf(pos, "%s", child->err);
                delete child;
                return result_fval = qnan;
            }
        }
    }
    
    float__t x_result = (a + b) * (float__t)0.5;
    
    // Step 4: Polish with parabolic interpolation
    for (i = 0; i < 10; i++)
    {
        float__t x1 = x_result - tol * (float__t)100.0;
        float__t x2 = x_result;
        float__t x3 = x_result + tol * (float__t)100.0;
        
        float__t f1, f2, f3;
        EVAL_F(x1, f1);
        EVAL_F(x2, f2);
        EVAL_F(x3, f3);
        
        // Parabolic interpolation
        float__t denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
        if (fabsl(denom) < tol)
            break;
        
        float__t A = x3 * (f2 - f1) + x2 * (f1 - f3) + x1 * (f3 - f2);
        float__t B = x3*x3 * (f1 - f2) + x2*x2 * (f3 - f1) + x1*x1 * (f2 - f3);
        
        if (fabsl(A) < tol)
            break;
        
        float__t x_new = B / ((float__t)2.0 * A);
        
        if (fabsl(x_new - x_result) < tol * fabsl(x_result))
            break;
        
        // Ensure we stay close
        if (fabsl(x_new - x_result) > fabsl(x_result) * (float__t)0.01)
            break;
        
        x_result = x_new;
    }
    
    #undef EVAL_F
    
    fflags |= child->isfflags();
    delete child;
    return x_result;
}