#ifndef	_MATH_H
#define	_MATH_H 1

#define INFINITY (1.f / 0.f)
#define NAN (0.f / 0.f)

#define isinf isinf_f64
#define isnan isnan_f64

#define fmodf mod_f32
#define fmod mod_f64

#define copysignf copysign_f32
#define copysign copysign_f64

#define fabsf fabs_f32
#define fabs fabs_f64

#define floorf floor_f32
#define floor floor_f64

#define ceilf ceil_f32
#define ceil ceil_f64

#define roundf round_f32
#define round round_f64

#define truncf trunc_f32
#define trunc trunc_f64

#define sinf sin_f32
#define sin sin_f64

#define cosf cos_f32
#define cos cos_f64

#define tanf tan_f32
#define tan tan_f64

#define asinf asin_f32
#define asin asin_f64

#define acosf acos_f32
#define acos acos_f64

#define atanf atan_f32
#define atan atan_f64

#define sinhf sinh_f32
#define sinh sinh_f64

#define coshf cosh_f32
#define cosh cosh_f64

#define tanhf tanh_f32
#define tanh tanh_f64

#define asinhf asinh_f32
#define asinh asinh_f64

#define acoshf acosh_f32
#define acosh acosh_f64

#define atanhf atanh_f32
#define atanh atanh_f64

#define atan2f atan2_f32
#define atan2 atan2_f64

#define fmaf fma_f32
#define fma fma_f64

#define sqrtf sqrt_f32
#define sqrt sqrt_f64

#define powf pow_f32
#define pow pow_f64

#define expf exp_f32
#define exp exp_f64

#define logf log_f32
#define log log_f64

#define exp2f exp2_f32
#define exp2 exp2_f64

#define log2f log2_f32
#define log2 log2_f64

#endif // _MATH_H

