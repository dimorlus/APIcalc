
#include <windows.h>
#include "pch.h"

#ifdef __BORLANDC__
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
#include <limits>

#else //__BORLANDC__

#include <cstdint>
#include <ctime>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#endif //__BORLANDC__

#include "scalc.h"
#include "sfmts.h"
#include "sfunc.h"

#include "ver.h"

#ifdef __BORLANDC__
#define M_PI_2l 1.5707963267948966192313216916398
#define PHI     1.6180339887498948482045868343656 //(1+sqrt(5))/2 golden ratio
#pragma warn -8004 // assigned a value that is never used

#else //__BORLANDC__

#define M_PI    3.1415926535897932384626433832795
#define M_PI_2l 1.5707963267948966192313216916398
#define M_E     2.7182818284590452353602874713527
#define PHI     1.6180339887498948482045868343656 //(1+sqrt(5))/2 golden ratio
#pragma warning(disable : 4996)
#pragma warning(disable : 4244)
#endif //__BORLANDC__

#define _WIN_
#define _ENABLE_PREIMAGINARY_

float__t Const (void *clc, char *name, float__t x)
{
 return ((calculator *)clc)->AddConst (name, x);
}

float__t Var (void *clc, char *name, float__t x)
{
 return ((calculator *)clc)->AddVar (name, x);
}

calculator::calculator (int cfg)
{
 errpos        = 0;
 pos           = 0;
 expr          = false;
 tmp_var_count = 0;
 err[0]        = '\0';
 scfg          = cfg;
 sres[0]       = '\0';
 memset (hash_table, 0, sizeof hash_table);
 memset (v_stack, 0, sizeof v_stack);
 c_imaginary = 'i';
 // randomize();
 srand (static_cast<unsigned int> (time (nullptr)));

 add (tsSFUNCF2, "const", (void *)Const);
 add (tsSFUNCF2, "var", (void *)Var);

 add (tsVFUNC1, vf_abs, "abs", (void *)vfunc);
 add (tsVFUNC1, vf_pol, "pol", (void *)vfunc);

 add (tsVFUNC1, vf_sin, "sin", (void *)vfunc);
 add (tsVFUNC1, vf_cos, "cos", (void *)vfunc);
 add (tsVFUNC1, vf_tan, "tan", (void *)vfunc);
 add (tsVFUNC1, vf_tan, "tg", (void *)vfunc);
 add (tsVFUNC1, vf_cot, "cot", (void *)vfunc);
 add (tsVFUNC1, vf_cot, "ctg", (void *)vfunc);

 add (tsVFUNC1, vf_sinh, "sinh", (void *)vfunc);
 add (tsVFUNC1, vf_sinh, "sh", (void *)vfunc);
 add (tsVFUNC1, vf_cosh, "cosh", (void *)vfunc);
 add (tsVFUNC1, vf_cosh, "ch", (void *)vfunc);
 add (tsVFUNC1, vf_tanh, "tanh", (void *)vfunc);
 add (tsVFUNC1, vf_tanh, "th", (void *)vfunc);
 add (tsVFUNC1, vf_ctnh, "ctanh", (void *)vfunc);
 add (tsVFUNC1, vf_ctnh, "cth", (void *)vfunc);

 add (tsVFUNC1, vf_asin, "asin", (void *)vfunc);
 add (tsVFUNC1, vf_asin, "arcsin", (void *)vfunc);
 add (tsVFUNC1, vf_acos, "acos", (void *)vfunc);
 add (tsVFUNC1, vf_acos, "arccos", (void *)vfunc);
 add (tsVFUNC1, vf_atan, "atan", (void *)vfunc);
 add (tsVFUNC1, vf_atan, "arctg", (void *)vfunc);
 add (tsVFUNC1, vf_acot, "acot", (void *)vfunc);
 add (tsVFUNC1, vf_acot, "arcctg", (void *)vfunc);

 add (tsVFUNC1, vf_asinh, "asinh", (void *)vfunc);
 add (tsVFUNC1, vf_asinh, "arsh", (void *)vfunc);
 add (tsVFUNC1, vf_acosh, "acosh", (void *)vfunc);
 add (tsVFUNC1, vf_acosh, "arch", (void *)vfunc);
 add (tsVFUNC1, vf_atanh, "atanh", (void *)vfunc);
 add (tsVFUNC1, vf_atanh, "arth", (void *)vfunc);
 add (tsVFUNC1, vf_acoth, "acoth", (void *)vfunc);
 add (tsVFUNC1, vf_acoth, "arcth", (void *)vfunc);

 add (tsVFUNC1, vf_exp, "exp", (void *)vfunc);
 add (tsVFUNC1, vf_log, "log", (void *)vfunc);
 add (tsVFUNC1, vf_log, "ln", (void *)vfunc);
 add (tsVFUNC1, vf_sqrt, "sqrt", (void *)vfunc);
 add (tsVFUNC1, vf_sqrt, "root2", (void *)vfunc);

 add (tsVFUNC2, vf_pow, "pow", (void *)vfunc2);
 add (tsVFUNC2, vf_rootn, "rootn", (void *)vfunc2);
 add (tsVFUNC2, vf_logn, "logn", (void *)vfunc2);

 add (tsVFUNC2, vf_cplx, "cmplx", (void *)vfunc2);
 add (tsVFUNC2, vf_cplx, "cplx", (void *)vfunc2);
 add (tsVFUNC2, vf_cplx, "cpx", (void *)vfunc2);
 add (tsVFUNC1, vf_re, "re", (void *)vfunc);
 add (tsVFUNC1, vf_im, "im", (void *)vfunc);

 add (tsFFUNC1, "erf", (void *)(float__t (*) (float__t))Erf);
 add (tsFFUNC2, "atan2", (void *)(float__t (*) (float__t, float__t))Atan2l);
 add (tsFFUNC2, "hypot", (void *)(float__t (*) (float__t, float__t))Hypot);
 add (tsFFUNC1, "log10", (void *)(float__t (*) (float__t))Lg);
 add (tsFFUNC1, "np", (void *)(float__t (*) (float__t))NP);
 add (tsFFUNC1, "db", (void *)(float__t (*) (float__t))DB);
 add (tsFFUNC1, "anp", (void *)(float__t (*) (float__t))ANP);
 add (tsFFUNC1, "adb", (void *)(float__t (*) (float__t))ADB);
 add (tsFFUNC1, "float", (void *)To_float);
 add (tsIFUNC1, "int", (void *)To_int);
 add (tsIFUNC2, "gcd", (void *)(int_t (*) (int_t, int_t))Gcd);
 add (tsIFUNC2, "invmod", (void *)(int_t (*) (int_t, int_t))Invmod);
 add (tsIFUNC1, "prime", (void *)Prime);
 add (tsPFUNCn, "fprn", (void *)(int_t (*) (char *, char *, int args, value *))fprn);
 add (tsPFUNCn, "prn", (void *)(int_t (*) (char *, char *, int args, value *))fprn);
 add (tsPFUNCn, "printf", (void *)(int_t (*) (char *, char *, int args, value *))fprn);
 add (tsSIFUNC1, "datatime", (void *)datatime);
 add (tsFFUNC1, "lg", (void *)(float__t (*) (float__t))Lg);
 add (tsFFUNC1, "exp10", (void *)(float__t (*) (float__t))Exp10);
 add (tsFFUNC1, "sing", (void *)(float__t (*) (float__t))Sing);
 add (tsFFUNC1, "cosg", (void *)(float__t (*) (float__t))Cosg);
 add (tsFFUNC1, "tgg", (void *)(float__t (*) (float__t))Tgg);
 add (tsFFUNC1, "ctgg", (void *)(float__t (*) (float__t))Ctgg);
 add (tsFFUNC1, "frac", (void *)(float__t (*) (float__t))Frac);
 add (tsFFUNC1, "round", (void *)(float__t (*) (float__t))Round);
 add (tsFFUNC1, "ceil", (void *)(float__t (*) (float__t))Ceil);
 add (tsFFUNC1, "floor", (void *)(float__t (*) (float__t))Floor);
 add (tsIFUNC1, "not", (void *)Not);
 add (tsIFUNC1, "now", (void *)Now);
 add (tsFFUNC2, "min", (void *)(float__t (*) (float__t, float__t))Min);
 add (tsFFUNC2, "max", (void *)(float__t (*) (float__t, float__t))Max);
 add (tsFFUNC1, "log2", (void *)(float__t (*) (float__t))Log2);
 add (tsFFUNC1, "fact", (void *)(float__t (*) (float__t))Factorial);
 add (tsFFUNC1, "frh", (void *)(float__t (*) (float__t))Farenheit);
 add (tsFFUNC1, "root3", (void *)(float__t (*) (float__t))Root3);
 add (tsFFUNC1, "cbrt", (void *)(float__t (*) (float__t))Root3);

 add (tsFFUNC1, "swg", (void *)(float__t (*) (float__t))Swg);
 add (tsFFUNC1, "sswg", (void *)(float__t (*) (float__t))SSwg);
 add (tsFFUNC1, "aswg", (void *)(float__t (*) (float__t))Aswg);
 add (tsFFUNC1, "awg", (void *)(float__t (*) (float__t))Awg);
 add (tsFFUNC1, "sawg", (void *)(float__t (*) (float__t))SAwg);
 add (tsFFUNC1, "aawg", (void *)(float__t (*) (float__t))Aawg);
 add (tsFFUNC1, "cs", (void *)(float__t (*) (float__t))Cs);
 add (tsFFUNC1, "acs", (void *)(float__t (*) (float__t))Acs);
 add (tsFFUNC1, "rnd", (void *)(float__t (*) (float__t))Random);
 add (tsFFUNC3, "vout", (void *)(float__t (*) (float__t, float__t, float__t))Vout);
 add (tsFFUNC3, "cmp", (void *)(float__t (*) (float__t, float__t, float__t))Cmp);
 add (tsFFUNC2, "ee", (void *)(float__t (*) (float__t, float__t))Ee);

 add (tsIFUNCF1, "wrgb", (void *)wavelength_to_rgb);
 add (tsIFUNCF1, "trgb", (void *)temperature_to_rgb);
 add (tsSFUNCF1, "winf", (void *)wavelength_info);

 // Mathematical constants
 addfconst ("pi", M_PI);
 addfconst ("e", M_E);
 addfconst ("phi", PHI);
 addfconst ("tau", 2.0 * M_PI);
 addfconst ("turn", 2.0 * M_PI);
 addfconst ("gon", M_PI / 200.0);
 addfconst ("deg", M_PI / 180.0);

 // Other imperial constants
 // Distance and length
 addfconst ("inch", 0.0254); // Inch (m)
 addfconst ("mil", 0.0000254); // 1/1000Inch (m)
 addfconst ("ft", 0.3048);   // Foot (m)
 addfconst ("yd", 0.9144);   // Yard (m)
 addfconst ("foot", 0.3048);   // Foot (m)
 addfconst ("yard", 0.9144);   // Yard (m)
 addfconst ("mi", 1609.344); // Mile (m)
 addfconst ("mile", 1609.344);   // Mile (m)
 addfconst ("nmi", 1852.0);    // Nautical mile
 addfconst ("ptt", 0.0254 / 72); // Point (m)

 // Mass 
 addfconst ("lb", 0.45359237); // Pound (kg)
 addfconst ("oz", 0.028349523125); // Ounce (kg)
 addfconst ("st", 6.35029318);      // Stone (kg)
 addfconst ("gr", 0.001);          // Gram (kg)
 addfconst ("kg", 1.0);          // Kilogram (kg)

// Volume
 addfconst ("gal", 0.003785411784); // US Gallon (m³)
 addfconst ("qt", 0.000946352946);  // US Quart (m³)
 addfconst ("pt", 0.000473176473);  // US Pint (m³)
 addfconst ("cup", 0.0002365882365); // US Cup (m³)
 addfconst ("floz", 2.95735295625e-5); // US Fluid Ounce (m³)
 addfconst ("tbsp", 1.478676478125e-5); // US Tablespoon (m³) 
 addfconst ("tsp", 4.92892159375e-6);   // US Teaspoon (m³)
 addfconst ("lt", 0.001);               // liters (m³)
 addfconst ("ml", 0.000001);            // milliliters (m³)
 addfconst ("cc", 0.000001);            // milliliters (m³)

 // Energy
 addfconst ("cal", 4.184);          // Calorie (J)
 addfconst ("kcal", 4184.0);        // Kilocalorie (J)
 addfconst ("btu", 1055.05585262);  // British thermal unit (J)
 addfconst ("wh", 3600.0);          // Watt hour (J)
 addfconst ("kwh", 3600000.0);      // Kilowatt hour (J)
 addfconst ("mwh", 3600000000.0);   // Megawatt hour (J)
 addfconst ("gtnt", 4184);          // Gram of TNT (Joules)
 addfconst ("ttnt", 4.184e9);       // Tonne of TNT (Joules)
 addfconst ("ktnt", 4.184e12);      // Kiloton of TNT (Joules)
 addfconst ("mtnt", 4.184e15);      // Megaton of TNT (Joules)
 addfconst ("ev", 1.602176634e-19); // Electronvolt (J)
 addfconst ("kev", 1.602176634e-16); // Kiloelectronvolt (J)
 addfconst ("mev", 1.602176634e-13); // Megaelectronvolt (J)
 addfconst ("gev", 1.602176634e-10); // Gigaelectronvolt (J)

 // Power
 addfconst ("hps", 745.69987158227022); // Horsepower (W)
 addfconst ("bhp", 745.69987158227022); // Brake horsepower (W) 
 
 // Pressure
 addfconst ("atm", 101325.0);          // Standard atmosphere (Pa)
 addfconst ("bar", 100000.0);          // Bar (Pa)
 addfconst ("psi", 6894.757293168361); // Pound-force per square inch (Pa)

 // Speed
 addfconst ("kmh", 0.277777778); // Kilometers per hour to meters per second
 addfconst ("mph", 0.44704);     // Miles per hour to meters per second   
 addfconst ("knot", 0.514444444); // Nautical miles per hour to meters per second

 // Time
 addfconst ("hour", 3600);        //Hour in seconds
 addfconst ("hr", 3600);          // Hour in seconds
 addfconst ("mnt", 60);           // Minute in seconds

 // Radiation units
 addfconst ("gy", 1.0);           // Gray (J/kg) - Base SI
 addfconst ("rad", 0.01);         // Rad (absorbed dose)
 addfconst ("sv", 1.0);           // Sievert (Equivalent dose) - Base SI
 addfconst ("rem", 0.01);         // Roentgen Equivalent Man (0.01 Sv)
 addfconst ("rn", 0.00877);       // Roentgen (approx in air)
 addfconst ("mrn", 0.00000877);   // Milli-roentgen
 addfconst ("urn", 0.00000000877); // Micro-roentgen
 addfconst ("ngnt", 3.6 * 0.00877); // Not great, not terrible (3.6 R)

 // Magnetic units
 addfconst ("tl", 1.0);  // Magnetic flux density (or magnetic induction) Tesla - Base SI 
 addfconst ("wb", 1.0);  // Magnetic flux Weber - Base SI
 addfconst ("gs", 1e-4); // Magnetic flux density (or magnetic induction) Gauss to Tesla
 addfconst ("mw", 1e-8); // Magnetic flux Maxwell to Weber
 addfconst ("oe", (1000.0 / (4.0 * M_PI))); // Magnetic field strength (H) Oersted to A/m
 addfconst ("gb", (10.0 / (4.0 * M_PI)));   // Magnetomotive force (MMF) Gilbert to Ampere-turn


 // Physical constants (CODATA 2018)
 // Fundamental constants
 addfconst ("c0", 299792458.0);     // Speed of light in vacuum (m/s)
 addfconst ("hp", 6.62607015e-34);  // Planck constant (J·s)
 addfconst ("hb", 1.054571817e-34); // Reduced Planck constant ℏ (J·s)
 addfconst ("gn", 6.67430e-11);     // Gravitational constant (m³/(kg·s²))
 addfconst ("na", 6.02214076e23);   // Avogadro constant (mol⁻¹)
 addfconst ("kb", 1.380649e-23);    // Boltzmann constant (J/K)
 addfconst ("rg", 8.314462618);     // Universal gas constant (J/(mol·K))
 addfconst ("sf", 5.670374419e-8);  // Stefan-Boltzmann constant (W/(m²·K⁴))
 addfconst ("rs", 8.314462618);     // Ideal gas constant kb*Na (J/(mol·K))
 addfconst ("nae", 2.1798723611035e-18); // Rydberg energy (J)
 addfconst ("mu", 1.66053906660e-27);    // Atomic mass constant (kg)

 addfconst ("stdt", 273.15);        // Standard temperature (K)
 
 // Electromagnetic constants
 addfconst ("e0", 8.8541878128e-12); // Electric constant, vacuum permittivity (F/m)
 addfconst ("u0", 1.25663706212e-6); // Magnetic constant, vacuum permeability (H/m)
 addfconst ("z0", 376.730313668);    // Characteristic impedance of vacuum (Ω)

 // Particle constants
 addfconst ("qe", 1.602176634e-19);   // Elementary charge (C)
 addfconst ("me", 9.1093837015e-31);  // Electron mass (kg)
 addfconst ("mp", 1.67262192369e-27); // Proton mass (kg)
 addfconst ("mn", 1.67492749804e-27); // Neutron mass (kg)
 addfconst ("rel", 2.8179403262e-15); // Classical electron radius (m)
 addfconst ("a0", 5.29177210903e-11); // Bohr radius (m)

 // Astronomical constants
 addfconst ("au", 1.495978707e11);     // Astronomical unit (m)
 addfconst ("ly", 9.4607304725808e15); // Light year (m)
 addfconst ("pc", 3.0856775814914e16); // Parsec (m)
 addfconst ("g0", 9.80665);            // Standard gravity (m/s²)

 // Additional constants
 addfconst ("ry", 10973731.568160); // Rydberg constant (m⁻¹)
 addfconst ("sb", 5.670374419e-8);  // Stefan-Boltzmann constant (W/(m²·K⁴))

 // Rainbow colors
 addfconst ("fir", 316e-6);
 addfconst ("lwir", 11.5e-6);
 addfconst ("mwir", 5.5e-6);
 addfconst ("swir", 2.2e-6);
 addfconst ("nir", 1.09e-6);
 addfconst ("red", 685e-9);
 addfconst ("orange", 605e-9);
 addfconst ("yellow", 580e-9);
 addfconst ("green", 532e-9);
 addfconst ("blue", 472e-9);
 addfconst ("indigo", 435e-9);
 addfconst ("violet", 400e-9);
 addfconst ("uva", 348e-9);
 addfconst ("uvb", 298e-9);
 addfconst ("uvc", 190e-9);

 // Integer Limits:
 addlvar ("max32", 2147483647.0, 0x7fffffff);
 addlvar ("maxint", 2147483647.0, 0x7fffffff);
 addlvar ("maxu32", 4294967295.0, 0xffffffff);
 addlvar ("maxuint", 4294967295.0, 0xffffffff);
 addlvar ("max64", 9223372036854775807.0, 0x7fffffffffffffffull);
 addlvar ("maxlong", 9223372036854775807.0, 0x7fffffffffffffffull);
 addlvar ("maxu64", 18446744073709551615.0, 0xffffffffffffffffull);
 addlvar ("maxulong", 18446744073709551615.0, 0xffffffffffffffffull);

 // System
 //  Get system timezone information
 TIME_ZONE_INFORMATION tzi;
 DWORD tzResult    = GetTimeZoneInformation (&tzi);
 long timezoneBias = tzi.Bias; // in minutes
 int daylight      = (tzResult == TIME_ZONE_ID_DAYLIGHT) ? 1 : 0;
 double tzHours    = -timezoneBias / 60.0;
 double currentTz  = tzHours + (daylight ? -tzi.DaylightBias / 60.0 : 0);

 addlvar ("timezone", tzHours, (int)tzHours);
 addlvar ("daylight", (float__t)daylight, daylight);
 addlvar ("tz", currentTz, (int)currentTz);
 addfconst ("version", _ver_);
 addim ();
}

calculator::~calculator (void)
{
 symbol *sp;
 symbol *nsp;

 for (int i = 0; i < hash_table_size; i++)
  {
   if ((sp = hash_table[i]) != nullptr)
    {
     do
      {
       nsp = sp->next;
       free (sp->name);
       sp->name = nullptr;
       delete sp;
       sp            = nsp;
       hash_table[i] = nullptr;
      }
     while (nsp);
    }
  }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int calculator::print (char *str, int Options, int binwide, int *size)
{
 int n     = 0;
 int bsize = 0;
 if (!expr)
  {
   bsize += sprintf (str + bsize, "%66.66s \r\n", " ");
   if (size) *size = bsize;
   n++;
   return n;
  }

 if (IsNaN (result_fval))
  {
   if (err[0])
    {
     int ep = errpos;
     if (ep < 0) ep = 0;
     if (ep > 0) ep--; // Перемещаем позицию ошибки на символ перед ней
     if ((ep < 64))
      {
       char binstr[80];
       memset (binstr, ' ', sizeof (binstr));
       memset (binstr, '-', ep);
       binstr[ep]                  = '^';
       binstr[sizeof (binstr) - 1] = '\0';
       bsize += sprintf (str + bsize, "%64.64s   \r\n", binstr);
       n++;
       bsize += sprintf (str + bsize, "%67.67s\r\n", err);
       n++;
      }
     else
      {
       bsize += sprintf (str + bsize, "%67.67s\r\n", err);
       n++;
      }
    }
   else
    {
     binwide = 8 * (binwide / 8);
     if (binwide < 8) binwide = 8;
     if (binwide > 64) binwide = 64;
     if (expr)
      bsize += sprintf (str + bsize, "%66.66s \r\n", "NaN");
     else
      bsize += sprintf (str + bsize, "%66.66s \r\n", " ");
     n++;

     // (RO) String format found
     if (((Options & STR) || (scfg & STR)) && (result_imval == 0))
      {
        {
         if (sres[0])
          {
           char strcstr[80];
           sprintf (strcstr, "'%s'", sres);
           bsize += sprintf (str + bsize, "%65.64s\r\n", strcstr);
           n++;
          }
         else
          {
           bsize += sprintf (str + bsize, "%65.64s S\r\n", "''");
           n++;
          }
        }
      }
    }
  }
 else
  {
   // (WO) Forced float
   if (Options & FFLOAT)
    {
     if (result_imval == 0)
      {
       bsize += sprintf (str + bsize, "%65.16Lg f\r\n", (long double)result_fval);
       n++;
      }
     else
      {
       char imstr[80];
       char cphi[20];
       float__t phi = atan2l (result_imval, result_fval);
       float__t r   = hypotl (result_fval, result_imval);
       dgr2str (cphi, phi);
       sprintf (imstr, "|%.8Lg|(%s) %.16Lg%+.16Lg%c", (long double)r, cphi,
                (long double)result_fval, (long double)result_imval, c_imaginary);

       //sprintf (imstr, "%.16Lg%+.16Lg%c", (long double)result_fval, (long double)result_imval,
       //         c_imaginary);
       bsize += sprintf (str + bsize, "%65.64s f\r\n", imstr);
       n++;
      }
    }
   // (RO) Scientific (6.8k) format found
   if (Options & (SCI | ENG))
    {
     char scistr[80];
     if (result_imval == 0)
      d2scistr (scistr, result_fval);
     else
      {
       char cphi[20];
       char cr[20];
       char *cp       = scistr;
       float__t imval = result_imval;
       float__t fval  = result_fval;
       float__t phi   = atan2l (imval, fval);
       float__t r     = hypotl (fval, imval);
       d2scistr (cr, r);
       dgr2str (cphi, phi);
       cp += sprintf (cp, "|%s|(%s) ", cr, cphi);
       normz (fval, imval);
       cp += d2scistr (cp, fval);
       if (imval >= 0) *cp++ = '+';

       cp += d2scistr (cp, imval);
       *cp++ = c_imaginary;
       *cp   = '\0';
      }
     bsize += sprintf (str + bsize, "%65.64s S\r\n", scistr);
     n++;
    }
   // (UI) Normalized output
   if (Options & NRM)
    {
     char nrmstr[80];
     float__t imval = result_imval;
     float__t fval  = result_fval;
     normz (fval, imval);

     if (imval == 0)
      d2nrmstr (nrmstr, fval);
     else
      {
       char cphi[20];
       char cr[20];
       char *cp       = nrmstr;
       float__t imval = result_imval;
       float__t fval  = result_fval;
       float__t phi   = atan2l (imval, fval);
       float__t r     = hypotl (fval, imval);
       d2nrmstr (cr, r);
       dgr2str (cphi, phi);
       cp += sprintf (cp, "|%s|(%s) ", cr, cphi);
       normz (fval, imval);
       cp += d2nrmstr (cp, fval);
       if (imval >= 0) *cp++ = '+';
       cp += d2nrmstr (cp, imval);
       *cp++ = c_imaginary;
       *cp   = '\0';
      }
     bsize += sprintf (str + bsize, "%65.64s n\r\n", nrmstr);
     n++;
    }

   // (RO) Computing format found
   if (((Options & CMP) || (scfg & CMP)) && (result_imval == 0))
    {
     char bscistr[80];
     b2scistr (bscistr, result_fval);
     bsize += sprintf (str + bsize, "%65.64s c\r\n", bscistr);
     n++;
    }

   // (UI) Integer output
   if ((Options & IGR) && (result_imval == 0))
    {
       bsize += sprintf (str + bsize, "%65lld i\r\n", result_ival);
       n++;
    }

   // (UI) Unsigned output
   if ((Options & UNS) && (result_imval == 0))
    {
       bsize += sprintf (str + bsize, "%65llu u\r\n", result_ival);//%llu|%zu
       n++;
    }

   // (UI) Fraction output
   if ((Options & FRC) && (result_imval == 0) && (result_tag == tvFLOAT))
    {
     char frcstr[80];
     int num, denum;
     double val;
     if (result_fval > 0)
      val = result_fval;
     else
      val = -result_fval;
     double intpart = floor (val);
     if (intpart < 1e15)
      {
       if (intpart > 0)
        {
         fraction (val - intpart, 0.001, num, denum);
         if (result_fval > 0)
          sprintf (frcstr, "%.0f+%d/%d", intpart, num, denum);
         else
          sprintf (frcstr, "-%.0f-%d/%d", intpart, num, denum);
        }
       else
        {
         fraction (val, 0.001, num, denum);
         if (result_fval > 0)
          sprintf (frcstr, "%d/%d", num, denum);
         else
          sprintf (frcstr, "-%d/%d", num, denum);
        }
       if (denum)
        {
         bsize += sprintf (str + bsize, "%65.64s F\r\n", frcstr);
         n++;
        }
      }
    }

   // (UI) Fraction inch output
   if ((Options & FRI) && (result_imval == 0) && (result_tag == tvFLOAT))
    {
     char frcstr[80];
     int num, denum;
     double val;
     if (result_fval > 0)
      val = result_fval;
     else
      val = -result_fval;
     val /= 25.4e-3;
     double intpart = floor (val);
     if (intpart < 1e15)
      {
       if (intpart > 0)
        {
         fraction (val - intpart, 0.001, num, denum);
         if (num && denum)
          {
           if (result_fval > 0)
            sprintf (frcstr, "%.0f+%d/%d", intpart, num, denum);
           else
            sprintf (frcstr, "-%.0f-%d/%d", intpart, num, denum);
          }
         else
          {
           sprintf (frcstr, "%.0f", intpart);
          }
        }
       else
        {
         fraction (val, 0.001, num, denum);
         if (result_fval > 0)
          sprintf (frcstr, "%d/%d", num, denum);
         else
          sprintf (frcstr, "-%d/%d", num, denum);
        }
       bsize += sprintf (str + bsize, "%65.64s \"\r\n", frcstr);
       n++;
      }
    }

   // (RO) Hex format found
   if (((Options & HEX) || (scfg & HEX)) && (result_imval == 0))
    {
     char binfstr[16];
     sprintf (binfstr, "%%64.%illxh  \r\n", binwide / 4);
      {
       bsize += sprintf (str + bsize, binfstr, result_ival);
       n++;
      }
    }

   // (RO) Octal format found
   if (((Options & OCT) || (scfg & OCT)) && (result_imval == 0))
    {
     char binfstr[16];
     sprintf (binfstr, "%%64.%illoo  \r\n", binwide / 3);
      {
       bsize += sprintf (str + bsize, binfstr, result_ival);
       n++;
      }
    }

   // (RO) Binary format found
   if (((Options & fBIN) || (scfg & fBIN)) && (result_imval == 0))
    {
     char binfstr[16];
     char binstr[80];
     sprintf (binfstr, "%%%ib", binwide);
     b2str (binstr, binfstr, result_ival);
      {
       bsize += sprintf (str + bsize, "%64.64sb  \r\n", binstr);
       n++;
      }
    }

   // (RO) Char format found
   if (((Options & CHR) || (scfg & CHR)) && (result_imval == 0))
    {
     char chrstr[80];
     chr2str (chrstr, result_ival);
      {
       bsize += sprintf (str + bsize, "%64.64s  c\r\n", chrstr);
       n++;
      }
    }

   // (RO) WChar format found
   if (((Options & WCH) || (scfg & WCH)) && (result_imval == 0))
    {
     char wchrstr[80];
     int i = result_ival & 0xffff;
     wchr2str (wchrstr, i);
      {
       bsize += sprintf (str + bsize, "%64.64s  c\r\n", wchrstr);
       n++;
      }
    }

   // (RO) Date time format found
   if (((Options & DAT) || (scfg & DAT)) && (result_imval == 0))
    {
     char dtstr[80];
     t2str (dtstr, result_ival);
      {
       bsize += sprintf (str + bsize, "%65.64s \r\n", dtstr);
       n++;
      }
    }

   // (RO) Unix time
   if (((Options & UTM) || (scfg & UTM)) && (result_imval == 0))
    {
     char dtstr[80];
     nx_time2str (dtstr, result_ival);
      {
       bsize += sprintf (str + bsize, "%65.64s  \r\n", dtstr);
       n++;
      }
    }

   // (RO) Degrees format found  * 180.0 / M_PI
   if (((Options & DEG) || (scfg & DEG)) && (result_imval == 0) && (result_tag == tvFLOAT))
    {
     char dgrstr[80];
     char *cp = dgrstr;
     cp += sprintf (cp, "%.6Lg rad|", (long double)result_fval);
     cp += dgr2str (cp, result_fval);
     cp += sprintf (cp, " (%.6Lg`)", (long double)result_fval * 180.0 / M_PI);
     cp += sprintf (cp, "|%.4Lg gon", (long double)result_fval * 200.0 / M_PI);
     cp += sprintf (cp, "|%.4Lg turn", (long double)result_fval * 0.5 / M_PI);

     bsize += sprintf (str + bsize, "%65.64s  \r\n", dgrstr);
     n++;
    }

   // (UI) Temperature format
   if (((Options & FRH) || (scfg & FRH)) && (result_imval == 0) && 
       (result_fval > -273.15) && (result_tag == tvFLOAT))
    {
     char frhstr[80];
     sprintf (frhstr, "%.6Lg K|%.6Lg `C|%.6Lg `F", (long double)(result_fval + 273.15),
              (long double)result_fval, (long double)(result_fval * 9.0 / 5.0 + 32.0));

     bsize += sprintf (str + bsize, "%65.64s  \r\n", frhstr);
     n++;
    }


   // (RO) String format found
   if (((Options & STR) || (scfg & STR)) && (result_imval == 0))
    {
      {
       if (sres[0])
        {
         char strcstr[80];
         sprintf (strcstr, "'%s'", sres);
         bsize += sprintf (str + bsize, "%65.64s S\r\n", strcstr);
         n++;
        }
       else
        {
         bsize += sprintf (str + bsize, "%65.64s S\r\n", "''");
         n++;
        }
      }
    }
  }
 if (size) *size = bsize;
 return n;
}
//---------------------------------------------------------------------------

int calculator::varlist (char *buf, int bsize, int *maxlen)
{
 char *cp = buf;
 symbol *sp;
 int lineCount = 0;
 int localMax  = 0;
 for (int i = 0; i < hash_table_size; i++)
  {
   if ((sp = hash_table[i]) != nullptr)
    {
     do
      {
       if (sp->tag == tsVARIABLE)
        {
         int written;
         if ((sp->val.tag == tvCOMPLEX) || (sp->val.imval != 0))
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = %-.5Lg%+.5Lgi\r\n", sp->name,
                               (float__t)sp->val.fval, (float__t)sp->val.imval);
          }
         else if (sp->val.tag == tvSTR)
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = \"%s\"\r\n", sp->name,
                               sp->val.sval ? sp->val.sval : "");
          }
         else
          {
           written = snprintf (cp, bsize - (cp - buf), "%-10s = %-.5Lg\r\n", sp->name,
                               (float__t)sp->val.fval);
          }
         if (written > localMax) localMax = written;
         cp += written;
         lineCount++;
        }
       sp = sp->next;
      }
     while (sp);
    }
  }
 if (maxlen) *maxlen = localMax;
 return lineCount;
}

unsigned calculator::string_hash_function (const char *p)
{
 unsigned h = 0, g;
 while (*p)
  {
   if (scfg & UPCASE)
    h = (h << 4) + tolower (*p++);
   else
    h = (h << 4) + *p++;

   if ((g = h & 0xF0000000) != 0)
    {
     h ^= g >> 24;
    }
   h &= ~g;
  }
 return h;
}

symbol *calculator::add (t_symbol tag, v_func fidx, const char *name, void *func)
{
 char *uname = strdup (name);

 unsigned h = string_hash_function (uname) % hash_table_size;
 symbol *sp;
 for (sp = hash_table[h]; sp != nullptr; sp = sp->next)
  {
   if (scfg & UPCASE)
    {
     if (stricmp (sp->name, uname) == 0) return sp;
    }
   else
    {
     if (strcmp (sp->name, uname) == 0) return sp;
    }
  }
 sp            = new symbol;
 sp->tag       = tag;
 sp->fidx      = fidx;
 sp->func      = func;
 sp->name      = uname;
 sp->val.tag   = tvINT;
 sp->val.ival  = 0;
 sp->next      = hash_table[h];
 hash_table[h] = sp;
 return sp;
}

symbol *calculator::add (t_symbol tag, const char *name, void *func)
{
 char *uname = strdup (name);

 unsigned h = string_hash_function (uname) % hash_table_size;
 symbol *sp;
 for (sp = hash_table[h]; sp != nullptr; sp = sp->next)
  {
   if (scfg & UPCASE)
    {
     if (stricmp (sp->name, uname) == 0) return sp;
    }
   else
    {
     if (strcmp (sp->name, uname) == 0) return sp;
    }
  }
 sp            = new symbol;
 sp->tag       = tag;
 sp->func      = func;
 sp->name      = uname;
 sp->val.tag   = tvINT;
 sp->val.ival  = 0;
 sp->next      = hash_table[h];
 hash_table[h] = sp;
 return sp;
}

float__t calculator::AddConst (const char *name, float__t val)
{
 if (find (name))
  {
   error ("constant redefinition");
   return std::numeric_limits<float__t>::quiet_NaN (); // 0.0/0.0;;
  } 
 addfconst (name, val);
 return val;
}

float__t calculator::AddVar (const char *name, float__t val)
{
 addfvar (name, val);
 return val;
}

symbol *calculator::find (const char *name)
{
 unsigned h = string_hash_function (name) % hash_table_size;
 symbol *sp;
 for (sp = hash_table[h]; sp != nullptr; sp = sp->next)
  {
   if (scfg & UPCASE)
    {
     if (stricmp (sp->name, name) == 0) return sp;
    }
   else
    {
     if (strcmp (sp->name, name) == 0) return sp;
    }
  }
 return nullptr;
}

void calculator::addfconst (const char *name, float__t val)
{
 symbol *sp   = add (tsCONSTANT, name);
 sp->val.tag  = tvFLOAT;
 sp->val.fval = val;
}

void calculator::addfvar (const char *name, float__t val)
{
 symbol *sp   = add (tsVARIABLE, name);
 sp->val.tag  = tvFLOAT;
 sp->val.fval = val;
}

void calculator::addim ()
{
#ifdef _ENABLE_PREIMAGINARY_
 symbol *sp = add(tsCONSTANT, "i");
 sp->val.tag       = tvCOMPLEX;
 sp->val.fval      = 0;
 sp->val.imval     = 1;
 sp                = add (tsCONSTANT, "j");
 sp->val.tag       = tvCOMPLEX;
 sp->val.fval      = 0;
 sp->val.imval     = 1;
#endif // _ENABLE_PREIMAGINARY_
}

void calculator::addivar (const char *name, int_t val)
{
 symbol *sp   = add (tsCONSTANT, name);
 sp->val.tag  = tvINT;
 sp->val.ival = val;
}

void calculator::addlvar (const char *name, float__t fval, int_t ival)
{
 symbol *sp   = add (tsCONSTANT, name);
 sp->val.tag  = tvINT;
 sp->val.fval = fval;
 sp->val.ival = ival;
}

int calculator::hscanf (char *str, int_t &ival, int &nn)
{
 int_t res = 0;
 char c;
 int n = 0;
 while (c = *str++, c && (n < 16))
  {
   if ((c >= '0') && (c <= '9'))
    {
     res = res * 16 + (c - '0');
     n++;
    }
   else if ((c >= 'A') && (c <= 'F'))
    {
     res = res * 16 + (c - 'A') + 0xA;
     n++;
    }
   else if ((c >= 'a') && (c <= 'f'))
    {
     res = res * 16 + (c - 'a') + 0xa;
     n++;
    }
   else
    break;
  }
 ival = res;
 nn   = n;
 if (n) scfg |= HEX;
 return 0;
}

int calculator::bscanf (char *str, int_t &ival, int &nn)
{
 int_t res = 0;
 char c;
 int n = 0;

 while (c = *str++, c && (n < 64))
  {
   if ((c >= '0') && (c <= '1'))
    {
     res = res * 2 + (c - '0');
     n++;
    }
   else
    break;
  }
 ival = res;
 nn   = n;
 if (n) scfg |= fBIN;
 return 0;
}

int calculator::oscanf (char *str, int_t &ival, int &nn)
{
 int_t res = 0;
 char c;
 int n = 0;

 while (c = *str++, c && (n < 24))
  {
   if ((c >= '0') && (c <= '7'))
    {
     res = res * 8 + (c - '0');
     n++;
    }
   else
    break;
  }
 ival = res;
 nn   = n;
 if (n) scfg |= OCT;
 return 0;
}

int calculator::xscanf (char *str, int len, int_t &ival, int &nn)
{
 int_t res = 0;
 char c;
 int n = 0;
 int hmax, omax;
 int max;

 switch (len)
  {
  case 1:
   max  = 0x100;
   hmax = 3;
   omax = 3;
   break;
  case 2:
   max  = 0x10000;
   hmax = 5;
   omax = 6;
   break;
  default:
   max = 0;
  }
 switch (*str)
  {
  case '0':
  case '1':
  case '2':
  case '3':
   {
    while (c = *str++, c && (n < omax))
     {
      if ((c >= '0') && (c <= '7'))
       {
        res = res * 8 + (c - '0');
        n++;
       }
      else
       break;
     }
    if (res >= max) n--;
    if (n) scfg |= OCT;
   }
   break;

  case 'x':
  case 'X':
   str++;
   n++;
   while (c = *str++, c && (res < max) && (n < hmax))
    {
     if ((c >= '0') && (c <= '9'))
      {
       res = res * 16 + (c - '0');
       n++;
      }
     else if ((c >= 'A') && (c <= 'F'))
      {
       res = res * 16 + (c - 'A') + 0xA;
       n++;
      }
     else if ((c >= 'a') && (c <= 'f'))
      {
       res = res * 16 + (c - 'a') + 0xa;
       n++;
      }
     else
      break;
    }
   if (res >= max) n--;
   if (n) scfg |= HEX;
   break;

  case 'a':
   res = '\007';
   n   = 1;
   scfg |= ESC;
   break;

  case 'f':
   res = 255u;
   n   = 1;
   scfg |= ESC;
   break;

  case 'v':
   res = '\x0b';
   n   = 1;
   scfg |= ESC;
   break;

  case 'E':
  case 'e':
   res = '\033';
   n   = 1;
   scfg |= ESC;
   break;

  case 't':
   res = '\t';
   n   = 1;
   scfg |= ESC;
   break;

  case 'n':
   res = '\n';
   n   = 1;
   scfg |= ESC;
   break;

  case 'r':
   res = '\r';
   n   = 1;
   scfg |= ESC;
   break;

  case 'b':
   res = '\b';
   n   = 1;
   scfg |= ESC;
   break;

  case '\\':
   res = '\\';
   n   = 1;
   scfg |= ESC;
   break;
  }
 ival = res;
 nn   = n;
 return 0;
}

float__t calculator::dstrtod (char *s, char **endptr)
{
 const char cdeg[]     = { '`', '\'', '\"' }; //` - degrees, ' - minutes, " - seconds
 const float__t mdeg[] = { M_PI / 180.0, M_PI / (180.0 * 60), M_PI / (180.0 * 60 * 60) };
 float__t res          = 0;
 float__t d;
 char *end = s;

 for (int i = 0; i < 3; i++)
  {
   d = strtod (end, &end);
   do
    {
     if (*end == cdeg[i])
      {
       res += d * mdeg[i];
       end++;
       scfg |= DEG;
       break;
      }
     else
      i++;
    }
   while (i < 3);
  }
 *endptr = end;
 return res;
}

// 1:c1:y1:d1:h1:m1:s  => 189377247661s
float__t calculator::tstrtod (char *s, char **endptr)
{
 const float__t dms[] = { (60.0 * 60.0 * 60.0 * 24.0 * 365.25 * 100.0),
                          (60.0 * 60.0 * 24.0 * 365.25),
                          (60.0 * 60.0 * 24.0 * 7),
                          (60.0 * 60.0 * 24.0),
                          (60.0 * 60.0),
                          60.0,
                          1.0 };
 const char cdt[]     = { 'c', 'y', 'w', 'd', 'h', 'm', 's' };
 float__t res         = 0;
 float__t d;
 char *end = s;

 for (int i = 0; i < 6; i++)
  {
   d = strtod (end, &end);
   do
    {
     if ((*end == ':') && (*(end + 1) == cdt[i]))
      {
       res += d * dms[i];
       end += 2;
       scfg |= DAT;
       break;
      }
     else
      i++;
    }
   while (i < 6);
  }
 *endptr = end;
 return res;
}

// https://en.wikipedia.org/wiki/Metric_prefix
// process expression like 1k56 => 1.56k (maximum 3 digits)
void calculator::engineering (float__t mul, char *&fpos, float__t &fval)
{
 int fract = 0;
 int div   = 1;
 int n     = 3; // maximum 3 digits
 while (*fpos && (*fpos >= '0') && ((*fpos <= '9')) && n--)
  {
   div *= 10;
   fract *= 10;
   fract += *fpos++ - '0';
   scfg |= ENG;
  }
 fval *= mul;
 fval += (fract * mul) / div;
}

bool calculator::isCMP (char *&fpos)
{
 if (*fpos == 'B')
  {
   fpos++;
   scfg |= CMP;
   return true;
  }
 else
 if ((*fpos == 'i') && (*(fpos + 1) == 'B'))
  {
   fpos += 2;
   scfg |= CMP;
   return true;
  }
 return false;
}

void calculator::scientific (char *&fpos, float__t &fval)
{
 if (*(fpos - 1) == 'E') fpos--;
 switch (*fpos)
  {
  case '\"': // Inch
   if (scfg & FRI)
    {
     fpos++;
     // fval *= 25.4e-3;
     engineering (25.4e-3, fpos, fval);
    }
   break;
  case 'Q':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.267650600228229401496703205376e+30; // 2**100
   else engineering (1e30, fpos, fval);
   break;
  case 'R':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.237940039285380274899124224e+27; // 2**90
   else engineering (1e27, fpos, fval);
   break;
  case 'Y':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.208925819614629174706176e+24; // 2**80
   else engineering (1e24, fpos, fval);
   break;
  case 'Z':
   fpos++;
   if (isCMP (fpos))
    fval *= 1.180591620717411303424e+21; // 2**70
   else engineering (1e21, fpos, fval);
   break;
  case 'E':
   fpos++;
   if (isCMP (fpos)) fval *= 1152921504606846976ull; // 2**60
   else engineering (1e18, fpos, fval);
   break;
  case 'P':
   fpos++;
   if (isCMP (fpos)) fval *= 1125899906842624ull; // 2**50
   else engineering (1e15, fpos, fval);
   break;
  case 'T':
   fpos++;
   if (isCMP (fpos)) fval *= 1099511627776ull; // 2**40
   else engineering (1e12, fpos, fval);
   break;
  case 'G':
   fpos++;
   if (isCMP (fpos)) fval *= 1073741824ull; // 2**30
   else engineering (1e9, fpos, fval);
   break;
  case 'M':
   fpos++;
   if (isCMP (fpos)) fval *= 1048576; // 2**20
   else engineering (1e6, fpos, fval);
   break;
  case 'K':
   fpos++;
   if (isCMP (fpos)) fval *= 1024; // 2**10
   else engineering (1e3, fpos, fval);
   break;
  //case 'R':
  // fpos++;
  // engineering (1, fpos, fval);
  // break;
  case 'h':
   fpos++;
   engineering (1e2, fpos, fval);
   break;
  case 'k':
   fpos++;
   engineering (1e3, fpos, fval);
   break;
  case 'D':
   fpos++;
   engineering (1e1, fpos, fval);
   break;
  case 'd':
   fpos++;
   if (*fpos == 'a')
    {
     fpos++;
     engineering (1e1, fpos, fval);
    }
   else
    engineering (1e-1, fpos, fval);
   break;
  case 'c':
   fpos++;
   engineering (1e-2, fpos, fval);
   break;
  case 'm':
   fpos++;
   engineering (1e-3, fpos, fval);
   break;
  case 'u':
   fpos++;
   engineering (1e-6, fpos, fval);
   break;
  case 'n':
   fpos++;
   engineering (1e-9, fpos, fval);
   break;
  case 'p':
   fpos++;
   engineering (1e-12, fpos, fval);
   break;
  case 'f':
   fpos++;
   engineering (1e-15, fpos, fval);
   break;
  case 'a':
   fpos++;
   engineering (1e-18, fpos, fval);
   break;
  case 'z':
   fpos++;
   engineering (1e-21, fpos, fval);
   break;
  case 'y':
   fpos++;
   engineering (1e-24, fpos, fval);
   break;
  case 'r':
   fpos++;
   engineering (1e-27, fpos, fval);
   break;
  case 'q':
   fpos++;
   engineering (1e-30, fpos, fval);
   break;
  }
}

void calculator::error (int pos, const char *msg)
{
 sprintf (err, "Error: %s at %i", msg, pos);
 errpos = pos;
}

static void SafeFree (value &v)
{
 if (v.tag == tvSTR && v.sval)
  {
   free (v.sval);
   v.sval = nullptr;
  }
}

static void DeepCopy (value &dest, const value &src)
{
 dest = src;
 if (src.tag == tvSTR && src.sval)
  {
   dest.sval = strdup (src.sval);
  }
}

t_operator calculator::scan (bool operand, bool percent)
{
 char name[max_expression_length], *np;

 while (isspace (buf[pos] & 0x7f)) pos += 1;
 switch (buf[pos++])
  {
  case '\0':
   return toEND;
  case '(':
   return toLPAR;
  case ')':
   return toRPAR;
  case '+':
   if (buf[pos] == '+')
    {
     pos += 1;
     return operand ? toPREINC : toPOSTINC;
    }
   else if (buf[pos] == '=')
    {
     pos += 1;
     return toSETADD;
    }
   return operand ? toPLUS : toADD;
  case '-':
   if (buf[pos] == '-')
    {
     pos += 1;
     return operand ? toPREDEC : toPOSTDEC;
    }
   else if (buf[pos] == '=')
    {
     pos += 1;
     return toSETSUB;
    }
   return operand ? toMINUS : toSUB;
  case '!':
   if (buf[pos] == '=')
    {
     pos += 1;
     return toNE;
    }
   return operand ? toNOT : toFACT;
  case '~':
   return toCOM;
  case ';':
   if (buf[pos] == ';')
    {
     pos += 1;
     return toEND;
    } 
   return toSEMI;
  case '*':
   if (buf[pos] == '*')
    {
     if (buf[pos + 1] == '=')
      {
       pos += 2;
       return toSETPOW;
      }
     pos += 1;
     return toPOW;
    }
   else if (buf[pos] == '=')
    {
     pos += 1;
     return toSETMUL;
    }
   return toMUL;
  case '/':
   if (buf[pos] == '=')
    {
     pos += 1;
     return toSETDIV;
    }
   else if (buf[pos] == '/')
    {
     pos += 1;
     return toPAR;
    }
   return toDIV;
  case '%':
   if (buf[pos] == '=')
    {
     pos += 1;
     return toSETMOD;
    }
   else if (buf[pos] == '%')
    {
     pos += 1;
     return toPERCENT;
    }
   return toMOD;
  case '<':
   if (buf[pos] == '<')
    {
     if (buf[pos + 1] == '=')
      {
       pos += 2;
       return toSETASL;
      }
     else
      {
       pos += 1;
       return toASL;
      }
    }
   else if (buf[pos] == '=')
    {
     pos += 1;
     return toLE;
    }
   else if (buf[pos] == '>')
    {
     pos += 1;
     return toNE;
    }
   return toLT;
  case '>':
   if (buf[pos] == '>')
    {
     if (buf[pos + 1] == '>')
      {
       if (buf[pos + 2] == '=')
        {
         pos += 3;
         return toSETLSR;
        }
       pos += 2;
       return toLSR;
      }
     else if (buf[pos + 1] == '=')
      {
       pos += 2;
       return toSETASR;
      }
     else
      {
       pos += 1;
       return toASR;
      }
    }
   else if (buf[pos] == '=')
    {
     pos += 1;
     return toGE;
    }
   return toGT;
  case '=':
   if (buf[pos] == '=')
    {
     scfg &= ~PAS;
     pos += 1;
     return toEQ;
    }
   if (scfg & PAS)
    return toEQ;
   else
    return toSET;
  case ':':
   if (buf[pos] == '=')
    {
     scfg |= PAS;
     pos += 1;
     return toSET;
    }
   error ("syntax error");
   return toERROR;
  case '&':
   if (buf[pos] == '&')
    {
     pos += 1;
     return toAND;
    }
   else if (buf[pos] == '=')
    {
     pos += 1;
     return toSETAND;
    }
   return toAND;
  case '|':
   if (buf[pos] == '|')
    {
     pos += 1;
     return toOR;
    }
   else if (buf[pos] == '=')
    {
     pos += 1;
     return toSETOR;
    }
   return toOR;
  case '^':
   if (scfg & PAS)
    {
     if (buf[pos] == '=')
      {
       pos += 1;
       return toSETPOW;
      }
     return toPOW;
    }
   else
    {
     if (buf[pos] == '=')
      {
       pos += 1;
       return toSETXOR;
      }
     return toXOR;
    }
  case '#':
   if (operand)
    {
     float__t fval;
     char *fpos;
     if (buf[pos])
      {
       fval                = Awg (strtod (buf + pos, &fpos));
       v_stack[v_sp].tag   = tvFLOAT;
       v_stack[v_sp].fval  = fval;
       pos                 = fpos - buf;
       v_stack[v_sp].pos   = pos;
       v_stack[v_sp++].var = nullptr;
       return toOPERAND;
      }
     else
      {
       error ("bad numeric constant");
       return toERROR;
      }
    }
   else
    {
     if (buf[pos] == '=')
      {
       pos += 1;
       return toSETXOR;
      }
     return toXOR;
    }
  case ',':
   return toCOMMA;
  case '\'':
   {
    int_t ival;
    char *ipos;
    int n = 0;

    if (buf[pos] == '\\')
     {
      xscanf (buf + pos + 1, 1, ival, n);
      ipos = buf + pos + n + 1;
      if (*ipos == '\'')
       ipos++;
      else
       {
        error ("bad char constant");
        return toERROR;
       }
     }
    else
     {
      ipos = buf + pos + 1;
      if (*ipos == '\'')
       {
#ifdef _WCHAR_
#ifdef _WIN_
        if (*(ipos + 1) == 'W')
         {
          wchar_t wbuf[2];
          char cbuf[2];

          cbuf[0] = *(ipos - 1);
          cbuf[1] = '\0';

          MultiByteToWideChar (CP_OEMCP, 0, (LPSTR)cbuf, -1, (LPWSTR)wbuf, 2);
          ival = *(int *)&wbuf[0];
          ipos += 2;
          scfg |= WCH;
         }
        else
#endif /*_WIN_*/
#endif /*_WCHAR_*/
         {
          scfg |= CHR;
          ival               = *(unsigned char *)(ipos - 1);
          v_stack[v_sp].sval = (char *)malloc (STRBUF);
          if (v_stack[v_sp].sval) v_stack[v_sp].sval[0] = *(ipos - 1);
          if (v_stack[v_sp].sval) v_stack[v_sp].sval[1] = '\0';
          ipos++;
         }
       }
      else
       {
        char sbuf[STRBUF];
        int sidx = 0;
        ipos     = buf + pos;
        while (*ipos && (*ipos != '\'') && (sidx < STRBUF)) sbuf[sidx++] = *ipos++;
        sbuf[sidx] = '\0';
        if (*ipos == '\'')
         {
          if (sbuf[0]) scfg |= STR;
          v_stack[v_sp].tag  = tvSTR;
          v_stack[v_sp].ival = 0;
          v_stack[v_sp].sval = (char *)malloc (STRBUF);
          if (v_stack[v_sp].sval) strcpy (v_stack[v_sp].sval, sbuf);
          pos                 = ipos - buf + 1;
          v_stack[v_sp].pos   = pos;
          v_stack[v_sp++].var = nullptr;
          return toOPERAND;
         }
        else
         {
          error ("bad char constant");
          return toERROR;
         }
       }
     }
    v_stack[v_sp].tag   = tvINT;
    v_stack[v_sp].ival  = ival;
    pos                 = ipos - buf;
    v_stack[v_sp].pos   = pos;
    v_stack[v_sp++].var = nullptr;
    return toOPERAND;
   }
#ifdef _WCHAR_
#ifdef _WIN_
  case 'L':
   {
    int_t ival;
    char *ipos;
    int n = 0;

    if (buf[pos] == '\'')
     {
      if (buf[pos + 1] == '\\')
       {
        xscanf (buf + pos + 2, 2, ival, n);
        ipos = buf + pos + n + 2;
        if (*ipos == '\'')
         ipos++;
        else
         {
          error ("bad char constant");
          return toERROR;
         }
       }
      else
       {
        ipos = buf + pos;
        if (*(ipos + 2) == '\'')
         {
          wchar_t wbuf[2];
          char cbuf[2];

          cbuf[0] = *(ipos + 1);
          cbuf[1] = '\0';

          MultiByteToWideChar (CP_OEMCP, 0, (LPSTR)cbuf, -1, (LPWSTR)wbuf, 2);
          ival = *(int *)&wbuf[0];
          ipos += 3;
          scfg |= WCH;
         }
        else
         {
          error ("bad char constant");
          return toERROR;
         }
       }
      v_stack[v_sp].tag   = tvINT;
      v_stack[v_sp].ival  = ival;
      pos                 = ipos - buf;
      v_stack[v_sp].pos   = pos;
      v_stack[v_sp++].var = nullptr;
      return toOPERAND;
     }
    goto def;
   }
#endif /*_WIN_*/
#endif /*_WCHAR_*/
  case '"':
   {
    char *ipos;
    char sbuf[STRBUF];
    int sidx = 0;
    ipos     = buf + pos;
    while (*ipos && (*ipos != '"') && (sidx < STRBUF - 1)) sbuf[sidx++] = *ipos++;
    sbuf[sidx] = '\0';
    if (*ipos == '"')
     {
      if (sbuf[0]) scfg |= STR;
      v_stack[v_sp].tag  = tvSTR;
      v_stack[v_sp].ival = 0;
      v_stack[v_sp].sval = (char *)malloc (STRBUF);
      if (v_stack[v_sp].sval) strcpy (v_stack[v_sp].sval, sbuf);
      pos                 = ipos - buf + 1;
      v_stack[v_sp].pos   = pos;
      v_stack[v_sp++].var = nullptr;
      return toOPERAND;
     }
    else
     {
      error ("bad char constant");
      return toERROR;
     }
   }
#ifdef _ENABLE_PREIMAGINARY_
 case 'i': 
 case 'j':
   {
    char *fpos;
    if (buf[pos] && (isdigit (buf[pos] & 0x7f) || buf[pos] == '.'))
     {
      float__t fval = strtod (buf + pos, &fpos);
      if (scfg & (ENG | SCI | FRI))
       {
        scientific (fpos, fval);
       }
      int ferr = errno;
      if ((ferr) && (*fpos == '.'))
       {
        pos = fpos - buf + 1;
        error ("bad numeric constant");
        return toERROR;
       }
      if (v_sp == max_stack_size)
       {
        error ("stack overflow");
        return toERROR;
       }

      c_imaginary         = buf[pos - 1];
      v_stack[v_sp].tag   = tvCOMPLEX;
      scfg |= CPX;
      v_stack[v_sp].imval = fval;
      v_stack[v_sp].fval  = 0;
      v_stack[v_sp].pos   = pos;
      v_stack[v_sp++].var = nullptr;
      pos                 = fpos - buf;
      return toOPERAND;
     }
    else
     goto def;
   }
#endif /*_ENABLE_PREIMAGINARY_*/
  case '.':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '\\':
  case '$':
   {
    int_t ival     = 0;
    float__t fval  = 0;
    float__t sfval = 0;
    int ierr       = 0, ferr;
    char *ipos, *fpos, *sfpos;
    int n = 0;

    if (buf[pos - 1] == '\\')
     {
      ierr = xscanf (buf + pos, 1, ival, n);
      ipos = buf + pos + n;
      scfg |= ESC;
     }
    else if ((buf[pos - 1] == '0') && ((buf[pos] == 'B') || (buf[pos] == 'b')))
     {
      ierr = bscanf (buf + pos + 1, ival, n);
      ipos = buf + pos + n + 1;
      scfg |= fBIN;
     }
    else if ((buf[pos - 1] == '0') && ((buf[pos] == 'O') || (buf[pos] == 'o')))
     {
      ierr = oscanf (buf + pos + 1, ival, n);
      ipos = buf + pos + n + 1;
      scfg |= OCT;
     }
    else if (buf[pos - 1] == '$')
     {
      ierr = hscanf (buf + pos, ival, n);
      ipos = buf + pos + n;
      scfg |= HEX;
     }
    else if ((buf[pos - 1] == '0') && ((buf[pos] == 'X') || (buf[pos] == 'x')))
     {
      ierr = hscanf (buf + pos + 1, ival, n);
      ipos = buf + pos + n + 1;
      scfg |= HEX;
     }
    else
     {
      errno = 0;
      #ifdef __BORLANDC__
      ival = strtol (buf + pos - 1, &ipos, 10);
      #else
      ival = strtoll (buf + pos - 1, &ipos, 10);
      #endif
      ierr = errno;
     }
    errno = 0;
    #ifdef __BORLANDC__
    sfval = fval = strtod (buf + pos - 1, &fpos);
    #else
    sfval = fval = strtold (buf + pos - 1, &fpos);
    #endif
    sfpos = fpos;

    v_stack[v_sp].tag = tvFLOAT;

    //` - degrees, ' - minutes, " - seconds
    if ((*fpos == '\'') || (*fpos == '`') || (((scfg & FRI) == 0) && (*fpos == '\"')))
     fval = dstrtod (buf + pos - 1, &fpos);
    else 
    if (*fpos == ':') fval = tstrtod (buf + pos - 1, &fpos);
    else 
    if (scfg & (ENG | SCI | FRI)) scientific (fpos, fval);
    if ((scfg & FRH) && (*fpos == 'F')) // Fahrenheit to Celsius
     {
      fpos++;
      if ((o_sp > 0) && (o_stack[o_sp - 1] == toMINUS))
       fval = -(-fval - 32.0) * 5.0 / 9.0;
      else
       fval = (fval - 32.0) * 5.0 / 9.0;
     }
    if (operand && percent && (*fpos == '%'))
     {
      fpos++;
      v_stack[v_sp].tag = tvPERCENT;
     }
    if ((*fpos == 'i') || (*fpos == 'j'))
     {
      c_imaginary = *fpos;
      fpos++;
      scfg |= CPX;
      v_stack[v_sp].tag = tvCOMPLEX;
     }
    if (*fpos && (isalnum (*fpos & 0x7f) || *fpos == '@' || *fpos == '_' || *fpos == '?'))
     {
      fpos = sfpos;
      fval = sfval;
     }
     
    if (v_stack[v_sp].tag == tvCOMPLEX)
     {
      v_stack[v_sp].imval = fval;
      v_stack[v_sp].fval  = 0;
     }
    else
     {
      v_stack[v_sp].fval  = fval;
      v_stack[v_sp].imval = 0;
     }
    pos = fpos - buf;

    if (v_stack[v_sp].tag == tvFLOAT)
     {
      ferr = errno;
      if ((ipos <= fpos) && ((*fpos == '.') || (*fpos == '$') || (*fpos == '\\')))
       {
        pos = fpos - buf + 1;
        error ("bad numeric constant");
        return toERROR;
       }
      if (ierr && ferr)
       {
        error ("bad numeric constant");
        return toERROR;
       }
      if (v_sp == max_stack_size)
       {
        error ("stack overflow");
        return toERROR;
       }
      if (!ierr && ipos >= fpos && 
          (*fpos != 'i') && (*fpos != 'j') && (*fpos != '%'))
       {
        if (scfg & FFLOAT) v_stack[v_sp].tag = tvFLOAT;
        else v_stack[v_sp].tag = tvINT;
        v_stack[v_sp].ival = ival;
        v_stack[v_sp].fval = (float__t)ival;
        pos                = ipos - buf;
       }
     }
    v_stack[v_sp].pos   = pos;
    v_stack[v_sp++].var = nullptr;
    return toOPERAND;
   }
  default:
  def:
   pos -= 1;
   np = name;
   while (isalnum (buf[pos] & 0x7f) || buf[pos] == '@' || buf[pos] == '_' || buf[pos] == '?')
    {
     *np++ = buf[pos++] & 0x7f;
    }
   if (np == buf)
    {
     error ("Bad character");
     return toERROR;
    }
   *np = '\0';
   symbol *sym = nullptr;
   if (name[0])
    {
     if (buf[pos] == '\0') sym = find (name);
     else sym = add (tsVARIABLE, name);
    }
   if (v_sp == max_stack_size)
    {
     error ("stack overflow");
     return toERROR;
    }
   if (sym)
    {
     DeepCopy (v_stack[v_sp], sym->val);
     v_stack[v_sp].pos   = pos;
     v_stack[v_sp++].var = sym;
     return (sym->tag == tsVARIABLE || sym->tag == tsCONSTANT) ? toOPERAND : toFUNC;
    }
   else
    return toOPERAND;
  }
}

static int lpr[toTERMINALS] = {
 2,  0,  0,  0,              // BEGIN, OPERAND, ERROR, END,
 4,  4,                      // LPAR, RPAR
 5,  98, 98, 98,             // FUNC, POSTINC, POSTDEC, FACT
 98, 98, 98, 98, 98, 98,     // PREINC, PREDEC, PLUS, MINUS, NOT, COM,
 90,                         // POW,
 80, 80, 80, 80, 80,         // toPERCENT, MUL, DIV, MOD, PAR
 70, 70,                     // ADD, SUB,
 60, 60, 60,                 // ASL, ASR, LSR,
 50, 50, 50, 50,             // GT, GE, LT, LE,
 40, 40,                     // EQ, NE,
 38,                         // AND,
 36,                         // XOR,
 34,                         // OR,
 20, 20, 20, 20, 20, 20, 20, // SET, SETADD, SETSUB, SETMUL, SETDIV, SETMOD,
 20, 20, 20, 20, 20, 20,     // SETASL, SETASR, SETLSR, SETAND, SETXOR, SETOR,
 8,                          // SEMI
 10                          // COMMA
};

static int rpr[toTERMINALS] = {
 0,   0,  0,  1,              // BEGIN, OPERAND, ERROR, END,
 110, 3,                      // LPAR, RPAR
 120, 99, 99, 99,             // FUNC, POSTINC, POSTDEC, FACT
 99,  99, 99, 99, 99, 99,     // PREINC, PREDEC, PLUS, MINUS, NOT, COM,
 95,                          // POW,
 80,  80, 80, 80, 80,         // toPERCENT, MUL, DIV, MOD, PAR
 70,  70,                     // ADD, SUB,
 60,  60, 60,                 // ASL, ASR, LSR,
 50,  50, 50, 50,             // GT, GE, LT, LE,
 40,  40,                     // EQ, NE,
 38,                          // AND,
 36,                          // XOR,
 34,                          // OR,
 25,  25, 25, 25, 25, 25, 25, // SET, SETADD, SETSUB, SETMUL, SETDIV, SETMOD,
 25,  25, 25, 25, 25, 25,     // SETASL, SETASR, SETLSR, SETAND, SETXOR, SETOR,
 10,                          // SEMI
 15                           // COMMA
};

bool calculator::assign ()
{
 value &v = v_stack[v_sp - 1];
 if (v.var == nullptr)
  {
   error (v.pos, "variable expected");
   return false;
  }
 else
  {
   if (v.var->tag == tsCONSTANT)
    {
     error (v.pos, "assignment to constant");
     return false;
    }
   SafeFree (v.var->val);
   DeepCopy (v.var->val, v);
   return true;
  }
}

void calculator::clear_v_stack ()
{
 // Очистка стека перед использованием
 for (int i = 0; i < max_stack_size; ++i)
  {
   if (v_stack[i].sval) free (v_stack[i].sval);
   v_stack[i].tag   = tvINT;
   v_stack[i].sval  = nullptr;
   v_stack[i].var   = nullptr;
   v_stack[i].pos   = 0;
   v_stack[i].ival  = 0;
   v_stack[i].fval  = 0.0;
   v_stack[i].imval = 0.0;
  }
 v_sp = 0;
}

float__t calculator::evaluate (char *expression, __int64 *piVal, float__t *pimval)
{
 char var_name[16];
 bool operand            = true;
 bool percent            = false;
 int n_args              = 0;
 const __int64 i64maxdbl = 0x7feffffffffffffeull;
 const __int64 i64mindbl = 0x0010000000000001ull;
 const double maxdbl     = *(double *)&i64maxdbl;
 const double mindbl     = *(double *)&i64mindbl;
 #ifdef __BORLANDC__
 const float__t qnan = 0.0/0.0;
 #else
 constexpr float__t qnan = std::numeric_limits<float__t>::quiet_NaN ();
 #endif
 t_operator saved_oper   = toBEGIN;
 value saved_val;
 bool has_saved_val = false;

 expr   = (expression && expression[0]);
 buf    = expression;
 v_sp   = 0;
 o_sp   = 0;
 pos    = 0;
 err[0] = '\0';
 result_fval  = qnan;
 result_imval = 0.0;
 result_ival = 0;   
 clear_v_stack ();

 if (!expr) return qnan;

 o_stack[o_sp++] = toBEGIN;

 memset (sres, 0, STRBUF);
 while (true)
  {
  next_token:
   int op_pos = pos;
   t_operator oper;
   if (has_saved_val)
    {
     v_stack[v_sp++] = saved_val;
     has_saved_val   = false;
    }
   if (saved_oper != toBEGIN)
    {
     oper       = saved_oper;
     saved_oper = toBEGIN;
    }
   else
    {
     oper = scan (operand, percent);
    }
   if (oper == toERROR)
    {
     result_fval = qnan;
     return qnan;
    }
  loper:
   switch (oper)
    {
    case toMUL:
    case toDIV:
    case toMOD:
    case toPOW:
    case toPAR:
    case toADD:
    case toSUB:
    case toCOMMA:
     percent = true;
     break;
    default:
     percent = false;
    }
   if (!operand)
    {
     if (!BINARY (oper) && oper != toEND && oper != toPOSTINC && oper != toPOSTDEC && oper != toRPAR
         && oper != toFACT)
      {
       if (scfg & IMUL)
        {
         // Implicit multiplication: cases like 2sin(x), 3(4+5), (1+2)(3+4)
         // Allow only if next token is: FUNC, LPAR, or OPERAND (not after scientific suffix)
         if (oper == toFUNC || oper == toLPAR || oper == toOPERAND)
          {
           saved_oper = oper;
           if (oper != toLPAR && v_sp > 0)
            {
             saved_val     = v_stack[--v_sp];
             has_saved_val = true;
            }
           oper = toMUL;
           goto loper;
          }
         else
          {
           error (op_pos, "operator expected");
           result_fval = qnan;  
           return qnan;
          }
        }
       else
        {
         error (op_pos, "operator expected");
         result_fval = qnan;
         return qnan;
        }
      }
     if (oper != toPOSTINC && oper != toPOSTDEC && oper != toRPAR && oper != toFACT)
      {
       operand = true;
      }
    }
   else
    {
     if (oper == toOPERAND)
      {
       operand = false;
       n_args += 1;
       continue;
      }
     if (BINARY (oper) || oper == toRPAR)
      {
       error (op_pos, "operand expected");
       result_fval = qnan;
       return qnan;
      }
    }
   n_args = 1;
   while (o_sp && (lpr[o_stack[o_sp - 1]] >= rpr[oper]))
    {
     t_operator cop = o_stack[--o_sp];
     if ((UNARY (cop) && (v_sp < 1)) || (BINARY (cop) && (v_sp < 2)))
      {
       error ("Unexpected end of expression");
       result_fval = qnan;
       return qnan;
      }

     switch (cop)
      {
      case toBEGIN:
       if (oper == toRPAR)
        {
         error ("Unmatched ')'");
         result_fval = qnan;
         return qnan;
        }
       if (oper != toEND) error ("Unexpected end of input");
       if (v_sp == 1)
        {
         if (scfg & UTMP)
          {
           sprintf (var_name, "@%d", ++tmp_var_count);
           add (tsVARIABLE, var_name)->val = v_stack[0];
          }
         result_fval = v_stack[0].get ();
         result_imval = v_stack[0].imval;
         result_ival  = v_stack[0].ival;
         result_tag   = v_stack[0].tag;
         if (piVal) *piVal = v_stack[0].ival;
         if (pimval) *pimval = v_stack[0].imval;
         if ((v_stack[0].tag == tvINT) && (v_stack[0].imval == 0.0))
          {
           result_ival = v_stack[0].ival;
           if (piVal) *piVal = v_stack[0].ival;
           if (pimval) *pimval = 0;
           if (v_stack[0].sval)
            {
             strcpy (sres, v_stack[0].sval);
             if (v_stack[0].sval) free (v_stack[0].sval);
             v_stack[0].sval = nullptr;
            }
           return v_stack[0].ival;
          }
         else
          {
           result_ival = (__int64)v_stack[0].fval;
           if (piVal) *piVal = (__int64)v_stack[0].fval;
           if (v_stack[0].sval)
            {
             strcpy (sres, v_stack[0].sval);
             if (v_stack[0].sval) free (v_stack[0].sval);
             v_stack[0].sval = nullptr;
            }
           else
            sres[0] = '\0';
           v_stack[0].imval = 0.0;
           v_stack[0].fval  = 0.0;
           v_stack[0].ival  = 0;
           v_stack[0].tag   = tvINT;
           return result_fval;
          }
        }
       else if (v_sp != 0)
       {
        error ("Unexpected end of expression");
        result_fval = qnan;
        return qnan;
       }
       else
       {
         //error ("Empty expression");
        expr        = false;
        result_fval = 0;
        return 0;
       }

      case toCOMMA: // ,
       n_args += 1;
       continue;

      case toSEMI: // ;
       if (((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
           || ((v_stack[v_sp - 1].imval != 0.0) || (v_stack[v_sp - 2].imval != 0.0)))
        {
         v_stack[v_sp - 2].fval  = v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].imval = v_stack[v_sp - 1].imval;
         v_stack[v_sp - 2].ival  = v_stack[v_sp - 1].ival;
         v_stack[v_sp - 2].tag   = tvCOMPLEX;
        }
       else if ((v_stack[v_sp - 1].tag == tvINT) && (v_stack[v_sp - 2].tag == tvINT))
        {
         v_stack[v_sp - 2].fval  = v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].imval = v_stack[v_sp - 1].imval;
         v_stack[v_sp - 2].ival  = v_stack[v_sp - 1].ival;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) && (v_stack[v_sp - 2].tag == tvSTR))
        {
         SafeFree (v_stack[v_sp - 2]);
         DeepCopy (v_stack[v_sp - 2], v_stack[v_sp - 1]);
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;   
         return qnan;
        }
       else
        {
         v_stack[v_sp - 2].fval = v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].tag  = tvFLOAT;
        }
       v_sp -= 1;
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toADD:    // +
      case toSETADD: // +=
       if ((v_stack[v_sp - 1].tag == tvINT) && (v_stack[v_sp - 2].tag == tvINT))
        {
         v_stack[v_sp - 2].ival += v_stack[v_sp - 1].ival;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) && (v_stack[v_sp - 2].tag == tvSTR))
        {
         if (strlen (v_stack[v_sp - 2].sval) + strlen (v_stack[v_sp - 1].sval) < STRBUF)
          {
           char *new_s = (char *)malloc (STRBUF);
           strcpy (new_s, v_stack[v_sp - 2].sval);
           strcat (new_s, v_stack[v_sp - 1].sval);
           SafeFree (v_stack[v_sp - 2]);
           v_stack[v_sp - 2].sval = new_s;
          }
         else
          {
           error (v_stack[v_sp - 2].pos, "String buffer overflow");
           result_fval = qnan;
           return qnan;
          }
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         v_stack[v_sp - 2].ival += v_stack[v_sp - 1].ival;
         v_stack[v_sp - 2].fval += v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].imval += v_stack[v_sp - 1].imval;
         v_stack[v_sp - 2].tag = tvCOMPLEX;
        }
       else
        {
         if (v_stack[v_sp - 1].tag == tvPERCENT)
          {
           float__t left          = v_stack[v_sp - 2].get ();
           float__t right         = v_stack[v_sp - 1].get ();
           v_stack[v_sp - 2].fval = left + (left * right / 100.0);
           v_stack[v_sp - 2].tag  = tvFLOAT;
          }
         else
          {
           v_stack[v_sp - 2].ival += v_stack[v_sp - 1].ival;
           v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () + v_stack[v_sp - 1].get ();
           v_stack[v_sp - 2].imval += v_stack[v_sp - 1].imval;
           v_stack[v_sp - 2].tag = tvFLOAT;
          }
         if (v_stack[v_sp - 2].imval != 0)
          v_stack[v_sp - 2].tag = tvCOMPLEX;
         else
          v_stack[v_sp - 2].tag = tvFLOAT;
        }
       v_sp -= 1;
       if (cop == toSETADD)
        {
         if (!assign ())
          {
           result_fval = qnan;
           return qnan;
          }
        }
       SafeFree (v_stack[v_sp]);
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toSUB:    // -
      case toSETSUB: // -=
       if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvINT) && (v_stack[v_sp - 2].tag == tvINT))
        {
         v_stack[v_sp - 2].ival -= v_stack[v_sp - 1].ival;
        }
       else if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         v_stack[v_sp - 2].ival -= v_stack[v_sp - 1].ival;
         v_stack[v_sp - 2].fval -= v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].imval -= v_stack[v_sp - 1].imval;
         v_stack[v_sp - 2].tag = tvCOMPLEX;
        }
       else
        {
         if (v_stack[v_sp - 1].tag == tvPERCENT)
          {
           float__t left          = v_stack[v_sp - 2].get ();
           float__t right         = v_stack[v_sp - 1].get ();
           v_stack[v_sp - 2].fval = left - (left * right / 100.0);
           v_stack[v_sp - 2].tag  = tvFLOAT;
          }
         else
          {
           v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () - v_stack[v_sp - 1].get ();
           v_stack[v_sp - 2].ival -= v_stack[v_sp - 1].ival;
           v_stack[v_sp - 2].imval -= v_stack[v_sp - 1].imval;
           v_stack[v_sp - 2].tag = tvFLOAT;
          }
         if (v_stack[v_sp - 2].imval != 0)
          v_stack[v_sp - 2].tag = tvCOMPLEX;
         else
          v_stack[v_sp - 2].tag = tvFLOAT;
        }
       v_sp -= 1;
       if (cop == toSETSUB)
        {
         if (!assign ()) 
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toMUL:    // *
      case toSETMUL: // *=
       if ((v_stack[v_sp - 1].tag == tvINT) && (v_stack[v_sp - 2].tag == tvINT))
        {
         v_stack[v_sp - 2].ival *= v_stack[v_sp - 1].ival;
         v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () * v_stack[v_sp - 1].get ();
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
                || ((v_stack[v_sp - 1].imval != 0.0) || (v_stack[v_sp - 2].imval != 0.0)))
        {
         // (a + bi) * (c + di) = (ac - bd) + (ad + bc)i

         long double a = v_stack[v_sp - 2].get ();
         long double b = v_stack[v_sp - 2].imval; //  a + bi
         long double c = v_stack[v_sp - 1].get ();
         long double d = v_stack[v_sp - 1].imval; //  c + di

         v_stack[v_sp - 2].fval  = a * c - b * d;
         v_stack[v_sp - 2].imval = a * d + b * c;
         v_stack[v_sp - 2].tag   = tvCOMPLEX;
        }
       else if (v_stack[v_sp - 2].tag != tvCOMPLEX)
        {
         if (v_stack[v_sp - 1].tag == tvPERCENT)
          {
           float__t left          = v_stack[v_sp - 2].get ();
           float__t right         = v_stack[v_sp - 1].get ();
           v_stack[v_sp - 2].fval = left * (left * right / 100.0);
           v_stack[v_sp - 2].tag  = tvFLOAT;
          }
         else
          {
           v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () * v_stack[v_sp - 1].get ();
          }
         v_stack[v_sp - 2].tag = tvFLOAT;
        }
       v_sp -= 1;
       if (cop == toSETMUL)
        {
         if (!assign ())
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toDIV:    // /
      case toSETDIV: // /=
       if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
                || ((v_stack[v_sp - 1].imval != 0.0) || (v_stack[v_sp - 2].imval != 0.0)))
        {
         // (a + bi) / (c + di) = [(ac + bd) + (bc - ad)i] / (c^2 + d^2)
         long double a     = v_stack[v_sp - 2].get ();
         long double b     = v_stack[v_sp - 2].imval;
         long double c     = v_stack[v_sp - 1].get ();
         long double d     = v_stack[v_sp - 1].imval;
         long double denom = c * c + d * d;
         if (denom == 0.0)
          {
           error (v_stack[v_sp - 2].pos, "Division by zero");
           result_fval = qnan;
           return qnan;
          }
         v_stack[v_sp - 2].fval  = (a * c + b * d) / denom;
         v_stack[v_sp - 2].imval = (b * c - a * d) / denom;
         v_stack[v_sp - 2].tag   = tvCOMPLEX;
        }
       else if (v_stack[v_sp - 1].get () == 0.0)
        {
         error (v_stack[v_sp - 2].pos, "Division by zero");
         result_fval = qnan;
         return qnan;
        }
       if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival /= v_stack[v_sp - 1].ival;
         v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () / v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       else if (v_stack[v_sp - 2].tag != tvCOMPLEX)
        {
         if (v_stack[v_sp - 1].tag == tvPERCENT)
          {
           float__t left          = v_stack[v_sp - 2].get ();
           float__t right         = v_stack[v_sp - 1].get ();
           v_stack[v_sp - 2].fval = left / (left * right / 100.0);
           v_stack[v_sp - 2].tag  = tvFLOAT;
          }
         else
          {
           v_stack[v_sp - 2].fval = v_stack[v_sp - 2].get () / v_stack[v_sp - 1].get ();
          }
         v_stack[v_sp - 2].tag = tvFLOAT;
        }
       v_sp -= 1;
       if (cop == toSETDIV)
        {
         if (!assign ())
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toPAR: // // parallel resistors
       if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].get () == 0.0) || (v_stack[v_sp - 2].get () == 0.0))
        {
         error (v_stack[v_sp - 2].pos, "Division by zero");
         result_fval = qnan;
         return qnan;
        }
       if (v_stack[v_sp - 1].tag == tvPERCENT)
        {
         float__t left          = v_stack[v_sp - 2].get ();
         float__t right         = v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].fval = 1 / (1 / left + 1 / (left * right / 100.0));
         v_stack[v_sp - 2].tag  = tvFLOAT;
        }
       else if (((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
                || ((v_stack[v_sp - 1].imval != 0.0) || (v_stack[v_sp - 2].imval != 0.0)))
        {
         long double ar = v_stack[v_sp - 2].get ();
         long double ai = v_stack[v_sp - 2].imval;
         long double br = v_stack[v_sp - 1].get ();
         long double bi = v_stack[v_sp - 1].imval;

         // 1/a
         long double a_norm2 = ar * ar + ai * ai;
         if (a_norm2 == 0.0)
          {
           error (v_stack[v_sp - 2].pos, "Division by zero");
           result_fval = qnan;
           return qnan;
          }
         long double inv_a_r = ar / a_norm2;
         long double inv_a_i = -ai / a_norm2;

         // 1/b
         long double b_norm2 = br * br + bi * bi;
         if (b_norm2 == 0.0)
          {
           error (v_stack[v_sp - 2].pos, "Division by zero");
           result_fval = qnan;
           return qnan;
          }
         long double inv_b_r = br / b_norm2;
         long double inv_b_i = -bi / b_norm2;

         // sum = 1/a + 1/b
         long double sum_r = inv_a_r + inv_b_r;
         long double sum_i = inv_a_i + inv_b_i;

         // 1 / sum
         long double sum_norm2 = sum_r * sum_r + sum_i * sum_i;
         if (sum_norm2 == 0.0)
          {
           error (v_stack[v_sp - 2].pos, "Division by zero");
           result_fval = qnan;
           return qnan;
          }
         v_stack[v_sp - 2].fval  = sum_r / sum_norm2;
         v_stack[v_sp - 2].imval = -sum_i / sum_norm2;
         v_stack[v_sp - 2].tag   = tvCOMPLEX;
        }
       else
        v_stack[v_sp - 2].fval = 1 / (1 / v_stack[v_sp - 1].get () + 1 / v_stack[v_sp - 2].get ());
       v_stack[v_sp - 2].tag = tvFLOAT;
       v_sp -= 1;
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toPERCENT:
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].get () == 0.0) || (v_stack[v_sp - 2].get () == 0.0))
        {
         error (v_stack[v_sp - 2].pos, "Division by zero");
         result_fval = qnan;
         return qnan;
        }
       if (v_stack[v_sp - 1].tag == tvPERCENT)
        {
         float__t left          = v_stack[v_sp - 2].get ();
         float__t right         = v_stack[v_sp - 1].get ();
         right                  = left * right / 100.0;
         v_stack[v_sp - 2].fval = 100.0 * (left - right) / right;
         v_stack[v_sp - 2].tag  = tvFLOAT;
        }
       else
        {
         float__t left          = v_stack[v_sp - 2].get ();
         float__t right         = v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].fval = 100.0 * (left - right) / right;
        }
       v_stack[v_sp - 2].tag = tvFLOAT;
       v_sp -= 1;
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toMOD:    // %
      case toSETMOD: // %=
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].get () == 0.0)
        {
         error (v_stack[v_sp - 2].pos, "Division by zero");
         result_fval = qnan;
         return qnan;
        }
       if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival %= v_stack[v_sp - 1].ival;
        }
       else
        {
         if (v_stack[v_sp - 1].tag == tvPERCENT)
          {
           float__t left          = v_stack[v_sp - 2].get ();
           float__t right         = v_stack[v_sp - 1].get ();
           v_stack[v_sp - 2].fval = fmod (left, left * right / 100.0);
           v_stack[v_sp - 2].tag  = tvFLOAT;
          }
         else
          v_stack[v_sp - 2].fval = fmod (v_stack[v_sp - 2].get (), v_stack[v_sp - 1].get ());
         v_stack[v_sp - 2].tag = tvFLOAT;
        }
       v_sp -= 1;
       if (cop == toSETMOD)
        {
         if (!assign ()) 
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toPOW:    // ** ^
      case toSETPOW: // **= ^=
       if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival
             = (int_t)pow ((float__t)v_stack[v_sp - 2].ival, (float__t)v_stack[v_sp - 1].ival);
        }
       else
        {
         if (v_stack[v_sp - 1].tag == tvPERCENT)
          {
           float__t left          = v_stack[v_sp - 2].get ();
           float__t right         = v_stack[v_sp - 1].get ();
           v_stack[v_sp - 2].fval = pow (left, left * right / 100.0);
           v_stack[v_sp - 2].tag  = tvFLOAT;
          }
         else if (((v_stack[v_sp - 2].tag == tvCOMPLEX) || (v_stack[v_sp - 1].tag == tvCOMPLEX))
                  || ((v_stack[v_sp - 1].imval != 0.0) || (v_stack[v_sp - 2].imval != 0.0)))
          {
           long double x1 = v_stack[v_sp - 2].get ();
           long double y1 = v_stack[v_sp - 2].imval; // x1 + i*y1
           long double x2 = v_stack[v_sp - 1].get ();
           long double y2 = v_stack[v_sp - 1].imval; // x2 + i*y2

           long double r    = std::hypotl (x1, y1);
           long double phi  = std::atan2 (y1, x1);
           long double ln_r = std::log (r);

           long double u = x2 * ln_r - y2 * phi;
           long double v = x2 * phi + y2 * ln_r;

           long double exp_u       = std::expl (u);
           v_stack[v_sp - 2].fval  = exp_u * std::cosl (v);
           v_stack[v_sp - 2].imval = exp_u * std::sinl (v);
           v_stack[v_sp - 2].tag   = tvCOMPLEX;
          }
         else
          {
           v_stack[v_sp - 2].fval = pow (v_stack[v_sp - 2].get (), v_stack[v_sp - 1].get ());
           v_stack[v_sp - 2].tag  = tvFLOAT;
          }
        }
       v_sp -= 1;
       if (cop == toSETPOW)
        {
         if (!assign ())
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toAND:    // &
      case toSETAND: // &=
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival &= v_stack[v_sp - 1].ival;
        }
       else
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () & v_stack[v_sp - 1].get_int ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       v_sp -= 1;
       if (cop == toSETAND)
        {
         if (!assign ()) 
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toOR:    // |
      case toSETOR: // |=
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival |= v_stack[v_sp - 1].ival;
        }
       else
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () | v_stack[v_sp - 1].get_int ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       v_sp -= 1;
       if (cop == toSETOR)
        {
         if (!assign ()) 
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toXOR:    // ^
      case toSETXOR: // ^=
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival ^= v_stack[v_sp - 1].ival;
        }
       else
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () ^ v_stack[v_sp - 1].get_int ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       v_sp -= 1;
       if (cop == toSETXOR)
        {
         if (!assign ())
          {
           result_fval = qnan;
           return qnan;
          }
         }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toASL:    // <<
      case toSETASL: // <<=
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival <<= v_stack[v_sp - 1].ival;
        }
       else
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () << v_stack[v_sp - 1].get_int ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       v_sp -= 1;
       if (cop == toSETASL)
        {
         if (!assign ()) 
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toASR:    // >>
      case toSETASR: // >>=
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival >>= v_stack[v_sp - 1].ival;
        }
       else
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get_int () >> v_stack[v_sp - 1].get_int ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       v_sp -= 1;
       if (cop == toSETASR)
        {
         if (!assign ()) 
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toLSR:    // >>> (logical shift right)
      case toSETLSR: // >>>=
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvSTR) || (v_stack[v_sp - 2].tag == tvSTR))
        {
         error (v_stack[v_sp - 2].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival = (unsigned_t)v_stack[v_sp - 2].ival >> v_stack[v_sp - 1].ival;
        }
       else
        {
         v_stack[v_sp - 2].ival = (unsigned_t)v_stack[v_sp - 2].get_int () >> v_stack[v_sp - 1].get_int ();
         v_stack[v_sp - 2].tag = tvINT;
        }
       v_sp -= 1;
       if (cop == toSETLSR)
        {
         if (!assign ()) 
          {
           result_fval = qnan;
           return qnan;
          }
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toEQ: // ==
       if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival == v_stack[v_sp - 1].ival;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
        {
         v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) == 0);
         v_stack[v_sp - 2].tag  = tvINT;
         SafeFree (v_stack[v_sp - 2]);
        }
       else
        {
         v_stack[v_sp - 2].ival = (v_stack[v_sp - 2].get () == v_stack[v_sp - 1].get ())
                                  && (v_stack[v_sp - 2].imval == v_stack[v_sp - 1].imval);
         v_stack[v_sp - 2].tag = tvINT;
        }
       v_sp -= 1;
       SafeFree (v_stack[v_sp - 1]);
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toNE: // !=, <>
       if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival != v_stack[v_sp - 1].ival;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
        {
         v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) != 0);
         v_stack[v_sp - 2].tag  = tvINT;
         SafeFree (v_stack[v_sp - 2]);
        }
       else
        {
         v_stack[v_sp - 2].ival = (v_stack[v_sp - 2].get () != v_stack[v_sp - 1].get ())
                                  || (v_stack[v_sp - 2].imval != v_stack[v_sp - 1].imval);
         v_stack[v_sp - 2].tag = tvINT;
        }
       v_sp -= 1;
       SafeFree (v_stack[v_sp - 1]);
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toGT: // >
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival > v_stack[v_sp - 1].ival;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
        {
         v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) > 0);
         v_stack[v_sp - 2].tag  = tvINT;
         SafeFree (v_stack[v_sp - 2]);
        }
       else
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get () > v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       v_sp -= 1;
       SafeFree (v_stack[v_sp - 1]);
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toGE: // >=
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival >= v_stack[v_sp - 1].ival;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
        {
         v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) >= 0);
         v_stack[v_sp - 2].tag  = tvINT;
         SafeFree (v_stack[v_sp - 2]);
        }
       else
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get () >= v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       v_sp -= 1;
       SafeFree (v_stack[v_sp - 1]);
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toLT: // <
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival < v_stack[v_sp - 1].ival;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
        {
         v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) < 0);
         v_stack[v_sp - 2].tag  = tvINT;
         SafeFree (v_stack[v_sp - 2]);
        }
       else
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get () < v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       v_sp -= 1;
       SafeFree (v_stack[v_sp - 1]);
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toLE: // <=
       if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 2].tag == tvCOMPLEX))
        {
         error (v_stack[v_sp - 2].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT && v_stack[v_sp - 2].tag == tvINT)
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].ival <= v_stack[v_sp - 1].ival;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR && v_stack[v_sp - 2].tag == tvSTR)
        {
         v_stack[v_sp - 2].ival = (strcmp (v_stack[v_sp - 2].sval, v_stack[v_sp - 1].sval) <= 0);
         v_stack[v_sp - 2].tag  = tvINT;
         free (v_stack[v_sp - 2].sval);
         v_stack[v_sp - 2].sval = nullptr;
        }
       else
        {
         v_stack[v_sp - 2].ival = v_stack[v_sp - 2].get () <= v_stack[v_sp - 1].get ();
         v_stack[v_sp - 2].tag  = tvINT;
        }
       v_sp -= 1;
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toPREINC: //++v
       if (v_stack[v_sp - 1].tag == tvCOMPLEX)
        {
         error (v_stack[v_sp - 1].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR)
        {
         error (v_stack[v_sp - 1].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT)
        {
         v_stack[v_sp - 1].ival += 1;
        }
       else
        {
         v_stack[v_sp - 1].fval += 1;
        }
       if (!assign ())
        {
         result_fval = qnan;
         return qnan;
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toPREDEC: // --v
       if (v_stack[v_sp - 1].tag == tvCOMPLEX)
        {
         error (v_stack[v_sp - 1].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR)
        {
         error (v_stack[v_sp - 1].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT)
        {
         v_stack[v_sp - 1].ival -= 1;
        }
       else
        {
         v_stack[v_sp - 1].fval -= 1;
        }
       if (!assign ()) 
        {
         result_fval = qnan;
         return qnan;
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toPOSTINC: // v++
       if (v_stack[v_sp - 1].tag == tvCOMPLEX)
        {
         error (v_stack[v_sp - 1].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR)
        {
         error (v_stack[v_sp - 1].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].var == nullptr)
        {
         error (v_stack[v_sp - 1].pos, "Varaibale expected");
         result_fval = qnan;
         return qnan;
        }
       if (v_stack[v_sp - 1].var->val.tag == tvINT)
        {
         v_stack[v_sp - 1].var->val.ival += 1;
        }
       else
        {
         v_stack[v_sp - 1].var->val.fval += 1;
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toPOSTDEC: // v--
       if (v_stack[v_sp - 1].tag == tvCOMPLEX)
        {
         error (v_stack[v_sp - 1].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR)
        {
         error (v_stack[v_sp - 1].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].var == nullptr)
        {
         error (v_stack[v_sp - 1].pos, "Varaibale expected");
         result_fval = qnan;
         return qnan;
        }
       if (v_stack[v_sp - 1].var->val.tag == tvINT)
        {
         v_stack[v_sp - 1].var->val.ival -= 1;
        }
       else
        {
         v_stack[v_sp - 1].var->val.fval -= 1;
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toFACT: // n!
       if (v_stack[v_sp - 1].tag == tvCOMPLEX)
        {
         error (v_stack[v_sp - 1].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR)
        {
         error (v_stack[v_sp - 1].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT)
        {
         v_stack[v_sp - 1].ival = (int_t)Factorial ((float__t)v_stack[v_sp - 1].ival);
        }
       else
        {
         v_stack[v_sp - 1].fval = (float__t)Factorial ((float__t)v_stack[v_sp - 1].fval);
        }
       v_stack[v_sp - 1].var = nullptr;

       break;

      case toSET: // =, :=
       if ((v_sp < 2) || (v_stack[v_sp - 2].var == nullptr))
        {
         if (v_sp < 2)
          error ("Variabale expected");
         else
          error (v_stack[v_sp - 2].pos, "Variabale expected");
         result_fval = qnan;
         return qnan;
        }
       else
        {
         if (v_stack[v_sp - 2].var->tag == tsCONSTANT)
          {
           error (v_stack[v_sp - 2].pos, "assignment to constant");
           result_fval = qnan;
           return qnan;
          }
         // v_stack[v_sp - 2] := v_stack[v_sp - 1]
         if ((v_stack[v_sp - 1].tag == tvSTR) && (v_stack[v_sp - 1].sval))
          {
           SafeFree (v_stack[v_sp - 2].var->val);
           DeepCopy (v_stack[v_sp - 2].var->val, v_stack[v_sp - 1]);
           SafeFree (v_stack[v_sp - 2]);
           DeepCopy (v_stack[v_sp - 2], v_stack[v_sp - 1]);
           v_stack[v_sp - 2].tag = tvSTR;
           SafeFree (v_stack[v_sp - 1]);
          }
         else
          v_stack[v_sp - 2] = v_stack[v_sp - 2].var->val = v_stack[v_sp - 1];
        }
       v_sp -= 1;
       // v_stack[v_sp-1].var = nullptr;
       break;

      case toNOT: // !
       if (v_stack[v_sp - 1].tag == tvCOMPLEX)
        {
         error (v_stack[v_sp - 1].pos, "Illegal complex operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvSTR)
        {
         error (v_stack[v_sp - 1].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if (v_stack[v_sp - 1].tag == tvINT)
        {
         v_stack[v_sp - 1].ival = (v_stack[v_sp - 1].ival == 0) ? 1 : 0;
        }
       else
        {
         v_stack[v_sp - 1].ival = (v_stack[v_sp - 1].fval == 0.0f) ? 1 : 0;
         v_stack[v_sp - 1].tag  = tvINT;
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toMINUS: // -v
       if ((v_stack[v_sp - 1].tag == tvSTR))
        {
         error (v_stack[v_sp - 1].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else
       {
        v_stack[v_sp - 1].ival = -v_stack[v_sp - 1].ival;
        v_stack[v_sp - 1].fval  = -v_stack[v_sp - 1].fval;
        v_stack[v_sp - 1].imval = -v_stack[v_sp - 1].imval;
       }
       //if (v_stack[v_sp - 1].tag == tvINT)
       // {
       //  v_stack[v_sp - 1].ival = -v_stack[v_sp - 1].ival;
       // }
       //else
       // {
       //  v_stack[v_sp - 1].fval  = -v_stack[v_sp - 1].fval;
       //  v_stack[v_sp - 1].imval = -v_stack[v_sp - 1].imval;
       // }
       // no break

      case toPLUS: //+v
       if ((v_stack[v_sp - 1].tag == tvSTR))
        {
         error (v_stack[v_sp - 1].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else
        v_stack[v_sp - 1].var = nullptr;
       break;

      case toCOM: // ~
       if (v_stack[v_sp - 1].tag == tvSTR)
        {
         error (v_stack[v_sp - 1].pos, "Illegal string operation");
         result_fval = qnan;
         return qnan;
        }
       else if ((v_stack[v_sp - 1].tag == tvCOMPLEX) || (v_stack[v_sp - 1].imval != 0.0))
        {
         v_stack[v_sp - 1].imval = -v_stack[v_sp - 1].imval;
         v_stack[v_sp - 1].tag   = tvCOMPLEX;
        }
       else if (v_stack[v_sp - 1].tag == tvINT)
        {
         v_stack[v_sp - 1].ival = ~v_stack[v_sp - 1].ival;
        }
       else
        {
         v_stack[v_sp - 1].ival = ~(uint64_t)v_stack[v_sp - 1].fval;
         v_stack[v_sp - 1].tag  = tvINT;
        }
       v_stack[v_sp - 1].var = nullptr;
       break;

      case toRPAR: // )
       error ("mismatched ')'");
       result_fval = qnan;
       return qnan;

      case toFUNC: // function without '('
       error ("'(' expected");
       result_fval = qnan;
       return qnan;

      case toLPAR: // (
       if (oper != toRPAR)
        {
         error ("')' expected");
         result_fval = qnan;
         return qnan;
        }

       if (o_stack[o_sp - 1] == toFUNC)
        {
         symbol *sym = v_stack[v_sp - n_args - 1].var;
         if (sym)
          {
           switch (sym->tag)
            {
            case tsVFUNC1: // float or complex f(x|z)
             if (n_args != 1)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
               result_fval = qnan;
               return qnan;
              }
             //if (scfg & CPX) v_stack[v_sp - 2].tag = tvCOMPLEX; // result should be complex
             ((void (*) (value *, value *, int))sym->func) (&v_stack[v_sp - 2], &v_stack[v_sp - 1],
                                                            sym->fidx);
             v_sp -= 1;
             break;

            case tsVFUNC2: // float or complex f(x|z,y|z)
             if (n_args != 2)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
               result_fval = qnan;
               return qnan;
              }
             //if (scfg & CPX) v_stack[v_sp - 3].tag = tvCOMPLEX; // result should be complex
             ((void (*) (value *, value *, value *, int))sym->func) (
                 &v_stack[v_sp - 3], &v_stack[v_sp - 2], &v_stack[v_sp - 1], sym->fidx);
             v_sp -= 2;
             break;

            case tsIFUNCF1: // int f(float x)
             if (n_args != 1)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
               result_fval = qnan;
               return qnan;
              }
             v_stack[v_sp - 2].ival = (*(int_t (*) (float__t))sym->func) (v_stack[v_sp - 1].get ());
             v_stack[v_sp - 2].tag  = tvINT;
             v_sp -= 1;
             break;

            case tsSFUNCF1: // char* f(float x)
             if (n_args != 1)
              {
               error (v_stack[v_sp - 2].pos, "Function should take one argument");
               result_fval = qnan;
               return qnan;
              }
             {
              const char *resStr
                  = (*(const char *(*)(float__t))sym->func) (v_stack[v_sp - 1].get ());
              strncpy (sres, resStr ? resStr : "", STRBUF - 1);
              sres[STRBUF - 1] = '\0';
              if (sres[0]) scfg |= STR;
              SafeFree (v_stack[v_sp - 2]);
              v_stack[v_sp - 2].sval = strdup (sres);
              v_stack[v_sp - 2].fval = v_stack[v_sp - 1].get ();
              v_stack[v_sp - 2].ival = v_stack[v_sp - 1].ival;
              v_stack[v_sp - 2].tag  = tvFLOAT; // tvINT;// tvSTR;
             }
             v_sp -= 1;
             break;

            case tsIFUNC1: // int f(int x)
             if (n_args != 1)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
               result_fval = qnan;
               return qnan;
              }
             v_stack[v_sp - 2].ival
                 = (*(int_t (*) (int_t))sym->func) (v_stack[v_sp - 1].get_int ());
             v_stack[v_sp - 2].tag = tvINT;
             v_sp -= 1;
             break;

            case tsIFUNC2: // int f(int x, int y)
             if (n_args != 2)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take two arguments");
               result_fval = qnan;
               return qnan;
              }
             v_stack[v_sp - 3].ival = (*(int_t (*) (int_t, int_t))sym->func) (
                 v_stack[v_sp - 2].get_int (), v_stack[v_sp - 1].get_int ());
             v_stack[v_sp - 3].tag = tvINT;
             v_sp -= 2;
             break;

            case tsIFFUNC3: // f(double, double, int)
             if (n_args != 3)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take three arguments");
               result_fval = qnan;
               return qnan;
              }
             v_stack[v_sp - 4].ival = (*(int_t (*) (double, double, int_t))sym->func) (
                 v_stack[v_sp - 3].get_dbl (), v_stack[v_sp - 2].get_dbl (),
                 v_stack[v_sp - 1].get_int ());
             v_stack[v_sp - 4].tag = tvINT;
             v_sp -= 3;
             break;

            case tsFFUNC1: // float f(float x)
             if (n_args != 1)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
               result_fval = qnan;
               return qnan;
              }
             v_stack[v_sp - 2].fval
                 = (*(float__t (*) (float__t))sym->func) (v_stack[v_sp - 1].get ());
             v_stack[v_sp - 2].tag = tvFLOAT;
             v_sp -= 1;
             break;

            case tsFFUNC2: // float f(float x, float y)
             if (n_args != 2)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take two arguments");
               result_fval = qnan;
               return qnan;
              }
             v_stack[v_sp - 3].fval = (*(float__t (*) (float__t, float__t))sym->func) (
                 v_stack[v_sp - 2].get (), v_stack[v_sp - 1].get ());
             v_stack[v_sp - 3].tag = tvFLOAT;
             v_sp -= 2;
             break;

            case tsFFUNC3: // float f(float x, float y, float z)
             if (n_args != 3)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take three arguments");
               result_fval = qnan;
               return qnan;
              }

             if (v_stack[v_sp - 1].tag == tvPERCENT) v_stack[v_sp - 1].fval /= 100;

             v_stack[v_sp - 4].fval = (*(float__t (*) (float__t, float__t, float__t))sym->func) (
                 v_stack[v_sp - 3].get (), v_stack[v_sp - 2].get (), v_stack[v_sp - 1].get ());
             v_stack[v_sp - 4].tag = tvFLOAT;
             v_sp -= 3;
             break;

            case tsPFUNCn: // f(str, ...)
             if (n_args < 1)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one or more arguments");
               result_fval = qnan;
               return qnan;
              }

             (*(int_t (*) (char *, char *, int, value *))sym->func) // call prn(...)
                 (sres, // put result string in sres first
                  v_stack[v_sp - n_args].get_str (), n_args - 1, &v_stack[v_sp - n_args + 1]);

             SafeFree (v_stack[v_sp - n_args - 1]);
             v_stack[v_sp - n_args - 1].sval = strdup (sres);
             v_stack[v_sp - n_args - 1].ival = 0;
             v_stack[v_sp - n_args - 1].tag  = tvINT; // tvSTR;

             if (n_args > 1)
              {
               v_stack[v_sp - n_args - 1].ival = v_stack[v_sp - n_args + 1].ival;
               if (v_stack[v_sp - n_args + 1].fval > maxdbl)
                v_stack[v_sp - n_args - 1].fval = qnan;
               else
                v_stack[v_sp - n_args - 1].fval = v_stack[v_sp - n_args + 1].fval;
              }
             v_sp -= n_args;
             break;

            case tsSFUNCF2: // f(str, x)
             if (n_args != 2)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take two arguments");
               result_fval = qnan;
               return qnan;
              }

             v_stack[v_sp - 3].fval = (*(float__t (*) (void*, char *, float__t))sym->func) // call const("name", value)
                ((void *) this, v_stack[v_sp - 2].get_str (), v_stack[v_sp - 1].get ());

             v_stack[v_sp - 3].tag = tvFLOAT;
             v_sp -= 2;
             break;

            case tsSIFUNC1: // int f(char *str)
             if (n_args != 1)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
               result_fval = qnan;
               return qnan;
              }
             v_stack[v_sp - 2].ival
                 = (*(int_t (*) (char *))sym->func) (v_stack[v_sp - 1].get_str ());
             v_stack[v_sp - 2].tag = tvINT;
             v_sp -= 1;
             break;
            case tsCFUNCC1: // f(x + iy)
             if (n_args != 1)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
               result_fval = qnan;
               return qnan;
              }
             {
              long double re = v_stack[v_sp - 1].fval;
              long double im = v_stack[v_sp - 1].imval;
              long double out_re, out_im;
              ((complex_func_t)sym->func) (re, im, out_re, out_im);
              v_stack[v_sp - 2].fval  = out_re;
              v_stack[v_sp - 2].imval = out_im;
              v_stack[v_sp - 2].tag   = tvCOMPLEX;
              v_stack[v_sp - 2].tag   = tvCOMPLEX;
             }
             v_sp -= 1;
             break;

            case tsFFUNCC1: // float f(x + iy)
             if (n_args != 1)
              {
               error (v_stack[v_sp - n_args - 1].pos, "Function should take one argument");
               result_fval = qnan;
               return qnan;
              }
             v_stack[v_sp - 2].fval = (*(float__t (*) (float__t, float__t))sym->func) (
                 v_stack[v_sp - 1].fval, v_stack[v_sp - 1].imval);
             v_stack[v_sp - 2].tag = tvFLOAT;
             v_sp -= 1;
             break;

            default:
             error ("Invalid expression");
            }
          }
         if (cop == toSETADD)
          {
           if (!assign ())
            {
             result_fval = qnan;
             return qnan;
            }
          }
         SafeFree (v_stack[v_sp - 1]);
         v_stack[v_sp - 1].var = nullptr;
         o_sp -= 1;
         n_args = 1;
        }
       else if (n_args != 1)
        {
         error ("Function call expected");
         result_fval = qnan;
         return qnan;
        }
       goto next_token;
      default:
       error ("syntax error");
      }
    }
   if (o_sp == max_stack_size)
    {
     error ("operator stack overflow");
     result_fval = qnan;
     return qnan;
    }
   o_stack[o_sp++] = oper;
  }
}
