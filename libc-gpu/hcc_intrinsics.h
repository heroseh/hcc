#ifndef	_HCC_INTRINSICS_H
#define	_HCC_INTRINSICS_H 1

#ifndef __HCC_GPU__
#error "this header is designed to only be used for the hcc compiler when targeting GPU"
#endif

#ifdef __HCC_GPU__
#define HCC_INTRINSIC __hcc_intrinsic
#else
#define HCC_INTRINSIC
#endif

#endif
