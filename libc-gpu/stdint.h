#ifndef _STDINT_H
#define _STDINT_H 1

#define INT8_MIN   (-128)
#define INT16_MIN  (-32767-1)
#define INT32_MIN  (-2147483647-1)
#define INT64_MIN  (-9223372036854775807-1)
#define INT8_MAX   127
#define INT16_MAX  32767
#define INT32_MAX  2147483647
#define INT64_MAX  9223372036854775807

#define UINT8_MAX  255u
#define UINT16_MAX 65535u
#define UINT32_MAX 4294967295u
#define UINT64_MAX 18446744073709551615u

typedef signed   char int8_t;
typedef unsigned char uint8_t;

#ifdef __HCC_X86_64__

#ifdef __HCC_LINUX__

typedef signed   short int16_t;
typedef unsigned short uint16_t;
typedef signed   int   int32_t;
typedef unsigned int   uint32_t;
typedef signed   long  int64_t;
typedef unsigned long  uint64_t;

typedef unsigned long  uintptr_t;
typedef signed long    intptr_t;

#elifdef __HCC_WINDOWS__

typedef signed   short     int16_t;
typedef unsigned short     uint16_t;
typedef signed   int       int32_t;
typedef unsigned int       uint32_t;
typedef signed   long long int64_t;
typedef unsigned long long uint64_t;

typedef unsigned long long uintptr_t;
typedef signed long   long intptr_t;

#else
#error "unsupported OS on the x86-64 host architecture"
#endif

#define INTPTR_MIN  (-9223372036854775807-1)
#define INTPTR_MAX  9223372036854775807
#define UINTPTR_MAX 18446744073709551615u

#else
#error "unsupported host architecture"
#endif

#endif // _STDINT_H
