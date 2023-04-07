#pragma once
#ifndef _HCC_FRAGMENT_INTRINSICS_H_
#define _HCC_FRAGMENT_INTRINSICS_H_

//
// returns the difference between 'v' on the threads in a quad along the X axis
// eg. (v in thread 1) - (v in thread 0)
// or  (v in thread 3) - (v in thread 2)
// will be ddx_fine_* or ddx_coarse_* based on external factors
float ddx_f32(float v);
f32x2 ddx_f32x2(f32x2 v);
f32x3 ddx_f32x3(f32x3 v);
f32x4 ddx_f32x4(f32x4 v);
#define ddxG(v) \
	_Generic((v), \
		float: ddx_f32, \
		f32x2: ddx_f32x2, \
		f32x3: ddx_f32x3, \
		f32x4: ddx_f32x4 \
	)(v)

//
// returns the difference between 'v' on the threads in a quad along the Y axis
// eg. (v in thread 0) - (v in thread 2)
// or  (v in thread 1) - (v in thread 3)
// will be ddy_fine_* or ddy_coarse_* based on external factors
float ddy_f32(float v);
f32x2 ddy_f32x2(f32x2 v);
f32x3 ddy_f32x3(f32x3 v);
f32x4 ddy_f32x4(f32x4 v);
#define ddyG(v) \
	_Generic((v), \
		float: ddy_f32, \
		f32x2: ddy_f32x2, \
		f32x3: ddy_f32x3, \
		f32x4: ddy_f32x4 \
	)(v)

//
// returns the abs(ddx_*(v)) + abs(ddy_*(v))
float fwidth_f32(float v);
f32x2 fwidth_f32x2(f32x2 v);
f32x3 fwidth_f32x3(f32x3 v);
f32x4 fwidth_f32x4(f32x4 v);
#define fwidthG(v) \
	_Generic((v), \
		float: fwidth_f32, \
		f32x2: fwidth_f32x2, \
		f32x3: fwidth_f32x3, \
		f32x4: fwidth_f32x4 \
	)(v)

//
// returns the difference between 'v' on the threads in a quad along the X axis
// with high precision actually using the top or bottom based on where the current thread is.
// eg. (v in thread 1) - (v in thread 0)
// or  (v in thread 3) - (v in thread 2)
float ddx_fine_f32(float v);
f32x2 ddx_fine_f32x2(f32x2 v);
f32x3 ddx_fine_f32x3(f32x3 v);
f32x4 ddx_fine_f32x4(f32x4 v);
#define ddx_fineG(v) \
	_Generic((v), \
		float: ddx_fine_f32, \
		f32x2: ddx_fine_f32x2, \
		f32x3: ddx_fine_f32x3, \
		f32x4: ddx_fine_f32x4 \
	)(v)

//
// returns the difference between 'v' on the threads in a quad along the Y axis
// with high precision actually using the left or right based on where the current thread is.
// eg. (v in thread 0) - (v in thread 2)
// or  (v in thread 1) - (v in thread 3)
float ddy_fine_f32(float v);
f32x2 ddy_fine_f32x2(f32x2 v);
f32x3 ddy_fine_f32x3(f32x3 v);
f32x4 ddy_fine_f32x4(f32x4 v);
#define ddy_fineG(v) \
	_Generic((v), \
		float: ddy_fine_f32, \
		f32x2: ddy_fine_f32x2, \
		f32x3: ddy_fine_f32x3, \
		f32x4: ddy_fine_f32x4 \
	)(v)

//
// returns the abs(ddx_fine_*(v)) + abs(ddy_fine_*(v))
float fwidth_fine_f32(float v);
f32x2 fwidth_fine_f32x2(f32x2 v);
f32x3 fwidth_fine_f32x3(f32x3 v);
f32x4 fwidth_fine_f32x4(f32x4 v);
#define fwidth_fineG(v) \
	_Generic((v), \
		float: fwidth_fine_f32, \
		f32x2: fwidth_fine_f32x2, \
		f32x3: fwidth_fine_f32x3, \
		f32x4: fwidth_fine_f32x4 \
	)(v)

//
// returns the difference between 'v' on the threads in a quad along the X axis
// with lower precision sharing the same value across all threads in a quad
// eg. (v in thread 1) - (v in thread 0)
float ddx_coarse_f32(float v);
f32x2 ddx_coarse_f32x2(f32x2 v);
f32x3 ddx_coarse_f32x3(f32x3 v);
f32x4 ddx_coarse_f32x4(f32x4 v);
#define ddx_coarseG(v) \
	_Generic((v), \
		float: ddx_coarse_f32, \
		f32x2: ddx_coarse_f32x2, \
		f32x3: ddx_coarse_f32x3, \
		f32x4: ddx_coarse_f32x4 \
	)(v)

//
// returns the difference between 'v' on the threads in a quad along the Y axis
// with lower precision sharing the same value across all threads in a quad
// eg. (v in thread 0) - (v in thread 2)
float ddy_coarse_f32(float v);
f32x2 ddy_coarse_f32x2(f32x2 v);
f32x3 ddy_coarse_f32x3(f32x3 v);
f32x4 ddy_coarse_f32x4(f32x4 v);
#define ddy_coarseG(v) \
	_Generic((v), \
		float: ddy_coarse_f32, \
		f32x2: ddy_coarse_f32x2, \
		f32x3: ddy_coarse_f32x3, \
		f32x4: ddy_coarse_f32x4 \
	)(v)

//
// returns the abs(ddx_coarse_*(v)) + abs(ddy_coarse_*(v))
float fwidth_coarse_f32(float v);
f32x2 fwidth_coarse_f32x2(f32x2 v);
f32x3 fwidth_coarse_f32x3(f32x3 v);
f32x4 fwidth_coarse_f32x4(f32x4 v);
#define fwidth_coarseG(v) \
	_Generic((v), \
		float: fwidth_coarse_f32, \
		f32x2: fwidth_coarse_f32x2, \
		f32x3: fwidth_coarse_f32x3, \
		f32x4: fwidth_coarse_f32x4 \
	)(v)

//
// discards the fragment so no writes will make it to output fragment render target and stops the fragment shader here.
// this will demote this thread to a helper thread so it will continue executing but will not write out to extern buffers or textures.
_Noreturn void discard_fragment(void);

#endif // _HCC_FRAGMENT_INTRINSICS_H_

