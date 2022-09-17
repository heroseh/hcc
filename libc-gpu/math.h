#ifndef	_MATH_H
#define	_MATH_H 1

#include "stdbool.h"

#define INFINITY (1.f / 0.f)
#define NAN (0.f / 0.f)
#define M_PI 3.14159265358979323846

HCC_INTRINSIC bool __hcc_isinf(__hcc_generic_float v);
HCC_INTRINSIC bool __hcc_isnan(__hcc_generic_float v);

#define isinf(v) __hcc_isinf(v)
#define isnan(v) __hcc_isnan(v)

HCC_INTRINSIC float fmodf(float a, float b);
HCC_INTRINSIC double fmod(double a, double b);

static inline float copysignf(float v, float sign) { return v * (sign < 0.f ? -1.f : 1.f); }
static inline double copysign(double v, double sign) { return v * (sign < 0.0 ? -1.0 : 1.0); }

HCC_INTRINSIC float fabsf(float v);
HCC_INTRINSIC double fabs(double v);

HCC_INTRINSIC float floorf(float v);
HCC_INTRINSIC double floor(double v);

HCC_INTRINSIC float ceilf(float v);
HCC_INTRINSIC double ceil(double v);

HCC_INTRINSIC float roundf(float v);
HCC_INTRINSIC double round(double v);

HCC_INTRINSIC float truncf(float v);
HCC_INTRINSIC double trunc(double v);

HCC_INTRINSIC float sinf(float v);
HCC_INTRINSIC double sin(double v);

HCC_INTRINSIC float cosf(float v);
HCC_INTRINSIC double cos(double v);

HCC_INTRINSIC float tanf(float v);
HCC_INTRINSIC double tan(double v);

HCC_INTRINSIC float asinf(float v);
HCC_INTRINSIC double asin(double v);

HCC_INTRINSIC float acosf(float v);
HCC_INTRINSIC double acos(double v);

HCC_INTRINSIC float atanf(float v);
HCC_INTRINSIC double atan(double v);

HCC_INTRINSIC float sinhf(float v);
HCC_INTRINSIC double sinh(double v);

HCC_INTRINSIC float coshf(float v);
HCC_INTRINSIC double cosh(double v);

HCC_INTRINSIC float tanhf(float v);
HCC_INTRINSIC double tanh(double v);

HCC_INTRINSIC float asinhf(float v);
HCC_INTRINSIC double asinh(double v);

HCC_INTRINSIC float acoshf(float v);
HCC_INTRINSIC double acosh(double v);

HCC_INTRINSIC float atanhf(float v);
HCC_INTRINSIC double atanh(double v);

HCC_INTRINSIC float atan2f(float y, float x);
HCC_INTRINSIC double atan2(double y, double x);

HCC_INTRINSIC float fmaf(float x, float y, float z);
HCC_INTRINSIC double fma(double x, double y, double z);

HCC_INTRINSIC float sqrtf(float v);
HCC_INTRINSIC double sqrt(double v);

HCC_INTRINSIC float powf(float v);
HCC_INTRINSIC double pow(double v);

HCC_INTRINSIC float expf(float v);
HCC_INTRINSIC double exp(double v);

HCC_INTRINSIC float logf(float v);
HCC_INTRINSIC double log(double v);

HCC_INTRINSIC float exp2f(float v);
HCC_INTRINSIC double exp2(double v);

HCC_INTRINSIC float log2f(float v);
HCC_INTRINSIC double log2(double v);

#endif // _MATH_H

