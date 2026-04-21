enum rtype { rtLin, rtExp, rtLg, rtPow, rtInv };

bool calculator::RegrFn(const char* fname, int n, rtype rt, value &res)
{
    FILE *f = nullptr;
    // Линеаризованные типы всегда имеют степень 1 (прямая y=a+bx)
    int degree = (rt == rtLin) ? n : 1;

    if (degree > 6) {
        error("degree too big (max 6)");
        return false;
    }

    // Инициализация накопителей во временном стеке
    int s_size = 2 * degree + 1;
    int sy_size = degree + 1;
    float__t *S = (float__t *)alloca(s_size * sizeof(float__t));
    float__t *SY = (float__t *)alloca(sy_size * sizeof(float__t));
    
    for (int i = 0; i < s_size; i++) S[i] = 0;
    for (int i = 0; i < sy_size; i++) SY[i] = 0;

    // 1. Потоковое чтение файла и накопление сумм
    if (fopen_s(&f, fname, "r") == 0 && f) {
        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            double v1 = qnan, v2 = qnan;
            // Твой strscan игнорирует мусор и понимает суффиксы (100k, 5m)
            if (strscan(line, 2, &v1, &v2) == 2) {
                float__t x = (float__t)v1;
                float__t y = (float__t)v2;

                // Линеаризация данных перед МНК
                switch (rt) {
                    case rtExp: if (y <= 0) continue; y = logl(y); break;
                    case rtLg:  if (x <= 0) continue; x = logl(x); break;
                    case rtPow: if (x <= 0 || y <= 0) continue; x = logl(x); y = logl(y); break;
                    case rtInv: if (x == 0) continue; x = 1.0 / x; break;
                    default: break; 
                }

                // Наполнение сумм степеней
                float__t px = 1.0;
                for (int i = 0; i < s_size; i++) {
                    S[i] += px;
                    if (i < sy_size) SY[i] += y * px;
                    px *= x;
                }
            }
        }
        fclose(f);
    } else {
        error("cannot open data file");
        return false;
    }

    // 2. Формирование матриц для решения системы M * A = B
    value valM, valB, valInvM, valCoeff;
    int dim = degree + 1;

    valM.tag = tvMATRIX; valM.mrows = dim; valM.mcols = dim;
    valM.mval = mxAlloc(dim, dim);
    
    valB.tag = tvMATRIX; valB.mrows = dim; valB.mcols = 1;
    valB.mval = mxAlloc(dim, 1);

    for (int r = 0; r < dim; r++) {
        for (int c = 0; c < dim; c++) {
            valM.mval[r * dim + c] = S[r + c];
        }
        valB.mval[r] = SY[r];
    }

    // 3. Решение системы через внутренние функции
    // A = inv(M) * B
    if (!mxInv(valInvM, valM)) {
        error("Matrix is singular (not enough distinct points?)");
        return false;
    }

    if (!mxMatMul(valCoeff, valInvM, valB)) return false;

    // 4. Постобработка и подготовка результата
    // mval для результата (матрица-строка 1 x n+1)
    float__t *res_mval = mxAlloc(1, dim);
    
    // Переносим коэффициенты в порядке от старшей степени к младшей (как в polynom)
    for (int i = 0; i < dim; i++) {
        res_mval[i] = valCoeff.mval[degree - i];
    }

    // Обратная трансформация для экспоненты и степени: ln(a) -> a
    if (rt == rtExp || rt == rtPow) {
        // Коэффициент 'a' у нас в конце массива res_mval (соответствует a0)
        res_mval[dim-1] = expl(res_mval[dim-1]);
    }

    res.tag = tvMATRIX;
    res.mrows = 1;
    res.mcols = dim;
    res.mval = res_mval;

    return true;
}