#ifndef __BORLANDC__
#include <cstdint>
#include <ctime>  // Добавляем заголовочный файл для работы с временем
#endif
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <errno.h>

#include "scalc.h"

int_t To_int(int_t val);
float__t To_float(float__t val);
int_t Prime(int_t n);
char* to_bin(int nval, char* pbuf, int nbufsize);
int_t Gcd(int_t x, int_t y);
int_t Invmod(int_t x, int_t y);
int_t Not(int_t n);
int_t Now(int_t n);
float__t Erf(float__t x);

float__t Erfc(float__t x);
float__t Random(float__t x);
float__t Atan(float__t x);
float__t Acot(float__t x);
float__t Atan2l(float__t x, float__t y);
float__t Cos(float__t x);
float__t Cosh(float__t x);
float__t Exp(float__t x);
float__t Sin(float__t x);
float__t Sinh(float__t x);
float__t Tan(float__t x);
float__t Tanh(float__t x);
float__t Pow(float__t x, float__t y);
float__t Sqrt(float__t x);
float__t Hypot(float__t x, float__t y);
float__t Acos(float__t x);
float__t Asin(float__t x);
float__t Log(float__t x);
float__t Lg(float__t x);
float__t NP(float__t x);
float__t DB(float__t x);
float__t Ctanh(float__t x);  //cth
float__t Arsh(float__t x);
float__t Arch(float__t x);
float__t Arth(float__t x);
float__t Arcth(float__t x);
float__t Exp10(float__t x);
float__t ANP(float__t x);
float__t ADB(float__t x);
float__t Sing(float__t x);
float__t Cosg(float__t x);
float__t Tgg(float__t x);
float__t Ctgg(float__t x);
float__t Ctg(float__t x);
float__t Round(float__t x);
float__t Ceil(float__t x);
float__t Floor(float__t x);
float__t Frac(float__t x);
float__t Min(float__t x, float__t y);
float__t Log2(float__t x);
float__t Logn(float__t x, float__t y);
float__t Root3(float__t x);
float__t Rootn(float__t x, float__t y);
float__t Swg(float__t x); //SWG to mm
float__t SSwg(float__t x); //SWG to mm^2
float__t Aswg(float__t x); //mm to SWG
float__t Awg(float__t x); //AWG to mm
float__t SAwg(float__t x); //AWG to mm^2
float__t Cs(float__t x); //diameter to mm^2
float__t Acs(float__t x); //mm^2 to diameter
float__t Aawg(float__t x); //mm to AWG
float__t Ee(float__t x, float__t y); //find standard value
float__t Max(float__t x, float__t y);
float__t Factorial(float__t x);
float__t Farenheit (float__t x);
float__t Cmp (float__t x, float__t y, float__t prec);
float__t Vout(float__t Vref, float__t Rh, float__t Rl);
bool IsNaN(const double fVal);
bool IsNaNL(const long double ldVal);
int_t fprn(char *dest, char *sfmt, int args, value* v_stack);
int_t datatime(char *tstr);

const char* wavelength_info(float__t wavelength_m);
uint32_t wavelength_to_rgb(float__t wavelength_m);
uint32_t temperature_to_rgb(float__t temp_kelvin);

float__t AbsC(float__t x, float__t y);
void SinC(float__t x, float__t y, float__t& re, float__t& im);
void CosC(float__t x, float__t y, float__t& re, float__t& im);
void TanC(float__t x, float__t y, float__t& re, float__t& im);
void CotC(float__t x, float__t y, float__t& re, float__t& im);

void AsinC(float__t x, float__t y, float__t& re, float__t& im);
void AcosC(float__t x, float__t y, float__t& re, float__t& im);
void AtanC(float__t x, float__t y, float__t& re, float__t& im);
void AcotC(float__t x, float__t y, float__t& re, float__t& im);

void SinhC(float__t x, float__t y, float__t& re, float__t& im);
void CoshC(float__t x, float__t y, float__t& re, float__t& im);
void TanhC(float__t x, float__t y, float__t& re, float__t& im);
void CothC(float__t x, float__t y, float__t& re, float__t& im);

void AsinhC(float__t x, float__t y, float__t& re, float__t& im);
void AcoshC(float__t x, float__t y, float__t& re, float__t& im);
void AtanhC(float__t x, float__t y, float__t& re, float__t& im);
void AcothC(float__t x, float__t y, float__t& re, float__t& im);

void ExpC(float__t x, float__t y, float__t& re, float__t& im);
void LnC(float__t x, float__t y, float__t& re, float__t& im);
void SqrtC(float__t x, float__t y, float__t& re, float__t& im);

void PowC(float__t x1, float__t y1, float__t x2, float__t y2, float__t& re, float__t& im);
void RootNC(float__t xr, float__t xi, float__t yr, float__t yi, float__t& re, float__t& im);
void LognC(float__t x, float__t y, float__t u, float__t v, float__t& re, float__t& im);

void vfunc(value* res, value* arg, int idx);
void vfunc2(value* res, value* arg1, value* arg2, int idx);
