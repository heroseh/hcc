#pragma once
#ifndef _HCC_WAVE_INTRINSICS_H_
#define _HCC_WAVE_INTRINSICS_H_

//
// swap 'v' with the thread across the X axis in the quad
// WARNING:
//     this is undefined behaviour when the swapping with an inactive thread.
//     this does not include helper threads so try to avoid using this function
//     in diverging control flow where all threads in the quad do not take the same branch.
//     unless you specifically know that the target thread will be active.
bool     quad_swap_x_bool(bool v);
half     quad_swap_x_f16(half v);
float    quad_swap_x_f32(float v);
double   quad_swap_x_f64(double v);
int8_t   quad_swap_x_s8(int8_t v);
int16_t  quad_swap_x_s16(int16_t v);
int32_t  quad_swap_x_s32(int32_t v);
int64_t  quad_swap_x_s64(int64_t v);
uint8_t  quad_swap_x_u8(uint8_t v);
uint16_t quad_swap_x_u16(uint16_t v);
uint32_t quad_swap_x_u32(uint32_t v);
uint64_t quad_swap_x_u64(uint64_t v);
boolx2   quad_swap_x_boolx2(boolx2 v);
f16x2    quad_swap_x_f16x2(f16x2 v);
f32x2    quad_swap_x_f32x2(f32x2 v);
f64x2    quad_swap_x_f64x2(f64x2 v);
s8x2     quad_swap_x_s8x2(s8x2 v);
s16x2    quad_swap_x_s16x2(s16x2 v);
s32x2    quad_swap_x_s32x2(s32x2 v);
s64x2    quad_swap_x_s64x2(s64x2 v);
u8x2     quad_swap_x_u8x2(u8x2 v);
u16x2    quad_swap_x_u16x2(u16x2 v);
u32x2    quad_swap_x_u32x2(u32x2 v);
u64x2    quad_swap_x_u64x2(u64x2 v);
boolx3   quad_swap_x_boolx3(boolx3 v);
f16x3    quad_swap_x_f16x3(f16x3 v);
f32x3    quad_swap_x_f32x3(f32x3 v);
f64x3    quad_swap_x_f64x3(f64x3 v);
s8x3     quad_swap_x_s8x3(s8x3 v);
s16x3    quad_swap_x_s16x3(s16x3 v);
s32x3    quad_swap_x_s32x3(s32x3 v);
s64x3    quad_swap_x_s64x3(s64x3 v);
u8x3     quad_swap_x_u8x3(u8x3 v);
u16x3    quad_swap_x_u16x3(u16x3 v);
u32x3    quad_swap_x_u32x3(u32x3 v);
u64x3    quad_swap_x_u64x3(u64x3 v);
boolx4   quad_swap_x_boolx4(boolx4 v);
f16x4    quad_swap_x_f16x4(f16x4 v);
f32x4    quad_swap_x_f32x4(f32x4 v);
f64x4    quad_swap_x_f64x4(f64x4 v);
s8x4     quad_swap_x_s8x4(s8x4 v);
s16x4    quad_swap_x_s16x4(s16x4 v);
s32x4    quad_swap_x_s32x4(s32x4 v);
s64x4    quad_swap_x_s64x4(s64x4 v);
u8x4     quad_swap_x_u8x4(u8x4 v);
u16x4    quad_swap_x_u16x4(u16x4 v);
u32x4    quad_swap_x_u32x4(u32x4 v);
u64x4    quad_swap_x_u64x4(u64x4 v);

//
// swap 'v' with the thread across the Y axis in the quad
// WARNING:
//     this is undefined behaviour when the swapping with an inactive thread.
//     this does not include helper threads so try to avoid using this function
//     in diverging control flow where all threads in the quad do not take the same branch.
//     unless you specifically know that the target thread will be active.
bool     quad_swap_y_bool(bool v);
half     quad_swap_y_f16(half v);
float    quad_swap_y_f32(float v);
double   quad_swap_y_f64(double v);
int8_t   quad_swap_y_s8(int8_t v);
int16_t  quad_swap_y_s16(int16_t v);
int32_t  quad_swap_y_s32(int32_t v);
int64_t  quad_swap_y_s64(int64_t v);
uint8_t  quad_swap_y_u8(uint8_t v);
uint16_t quad_swap_y_u16(uint16_t v);
uint32_t quad_swap_y_u32(uint32_t v);
uint64_t quad_swap_y_u64(uint64_t v);
boolx2   quad_swap_y_boolx2(boolx2 v);
f16x2    quad_swap_y_f16x2(f16x2 v);
f32x2    quad_swap_y_f32x2(f32x2 v);
f64x2    quad_swap_y_f64x2(f64x2 v);
s8x2     quad_swap_y_s8x2(s8x2 v);
s16x2    quad_swap_y_s16x2(s16x2 v);
s32x2    quad_swap_y_s32x2(s32x2 v);
s64x2    quad_swap_y_s64x2(s64x2 v);
u8x2     quad_swap_y_u8x2(u8x2 v);
u16x2    quad_swap_y_u16x2(u16x2 v);
u32x2    quad_swap_y_u32x2(u32x2 v);
u64x2    quad_swap_y_u64x2(u64x2 v);
boolx3   quad_swap_y_boolx3(boolx3 v);
f16x3    quad_swap_y_f16x3(f16x3 v);
f32x3    quad_swap_y_f32x3(f32x3 v);
f64x3    quad_swap_y_f64x3(f64x3 v);
s8x3     quad_swap_y_s8x3(s8x3 v);
s16x3    quad_swap_y_s16x3(s16x3 v);
s32x3    quad_swap_y_s32x3(s32x3 v);
s64x3    quad_swap_y_s64x3(s64x3 v);
u8x3     quad_swap_y_u8x3(u8x3 v);
u16x3    quad_swap_y_u16x3(u16x3 v);
u32x3    quad_swap_y_u32x3(u32x3 v);
u64x3    quad_swap_y_u64x3(u64x3 v);
boolx4   quad_swap_y_boolx4(boolx4 v);
f16x4    quad_swap_y_f16x4(f16x4 v);
f32x4    quad_swap_y_f32x4(f32x4 v);
f64x4    quad_swap_y_f64x4(f64x4 v);
s8x4     quad_swap_y_s8x4(s8x4 v);
s16x4    quad_swap_y_s16x4(s16x4 v);
s32x4    quad_swap_y_s32x4(s32x4 v);
s64x4    quad_swap_y_s64x4(s64x4 v);
u8x4     quad_swap_y_u8x4(u8x4 v);
u16x4    quad_swap_y_u16x4(u16x4 v);
u32x4    quad_swap_y_u32x4(u32x4 v);
u64x4    quad_swap_y_u64x4(u64x4 v);

//
// swap 'v' with the thread across diagonally in the quad
// WARNING:
//     this is undefined behaviour when the swapping with an inactive thread.
//     this does not include helper threads so try to avoid using this function
//     in diverging control flow where all threads in the quad do not take the same branch.
//     unless you specifically know that the target thread will be active.
bool     quad_swap_diagonal_bool(bool v);
half     quad_swap_diagonal_f16(half v);
float    quad_swap_diagonal_f32(float v);
double   quad_swap_diagonal_f64(double v);
int8_t   quad_swap_diagonal_s8(int8_t v);
int16_t  quad_swap_diagonal_s16(int16_t v);
int32_t  quad_swap_diagonal_s32(int32_t v);
int64_t  quad_swap_diagonal_s64(int64_t v);
uint8_t  quad_swap_diagonal_u8(uint8_t v);
uint16_t quad_swap_diagonal_u16(uint16_t v);
uint32_t quad_swap_diagonal_u32(uint32_t v);
uint64_t quad_swap_diagonal_u64(uint64_t v);
boolx2   quad_swap_diagonal_boolx2(boolx2 v);
f16x2    quad_swap_diagonal_f16x2(f16x2 v);
f32x2    quad_swap_diagonal_f32x2(f32x2 v);
f64x2    quad_swap_diagonal_f64x2(f64x2 v);
s8x2     quad_swap_diagonal_s8x2(s8x2 v);
s16x2    quad_swap_diagonal_s16x2(s16x2 v);
s32x2    quad_swap_diagonal_s32x2(s32x2 v);
s64x2    quad_swap_diagonal_s64x2(s64x2 v);
u8x2     quad_swap_diagonal_u8x2(u8x2 v);
u16x2    quad_swap_diagonal_u16x2(u16x2 v);
u32x2    quad_swap_diagonal_u32x2(u32x2 v);
u64x2    quad_swap_diagonal_u64x2(u64x2 v);
boolx3   quad_swap_diagonal_boolx3(boolx3 v);
f16x3    quad_swap_diagonal_f16x3(f16x3 v);
f32x3    quad_swap_diagonal_f32x3(f32x3 v);
f64x3    quad_swap_diagonal_f64x3(f64x3 v);
s8x3     quad_swap_diagonal_s8x3(s8x3 v);
s16x3    quad_swap_diagonal_s16x3(s16x3 v);
s32x3    quad_swap_diagonal_s32x3(s32x3 v);
s64x3    quad_swap_diagonal_s64x3(s64x3 v);
u8x3     quad_swap_diagonal_u8x3(u8x3 v);
u16x3    quad_swap_diagonal_u16x3(u16x3 v);
u32x3    quad_swap_diagonal_u32x3(u32x3 v);
u64x3    quad_swap_diagonal_u64x3(u64x3 v);
boolx4   quad_swap_diagonal_boolx4(boolx4 v);
f16x4    quad_swap_diagonal_f16x4(f16x4 v);
f32x4    quad_swap_diagonal_f32x4(f32x4 v);
f64x4    quad_swap_diagonal_f64x4(f64x4 v);
s8x4     quad_swap_diagonal_s8x4(s8x4 v);
s16x4    quad_swap_diagonal_s16x4(s16x4 v);
s32x4    quad_swap_diagonal_s32x4(s32x4 v);
s64x4    quad_swap_diagonal_s64x4(s64x4 v);
u8x4     quad_swap_diagonal_u8x4(u8x4 v);
u16x4    quad_swap_diagonal_u16x4(u16x4 v);
u32x4    quad_swap_diagonal_u32x4(u32x4 v);
u64x4    quad_swap_diagonal_u64x4(u64x4 v);

//
// read 'v' from a thread at 'thread_idx' in the quad
// WARNING:
//     this is undefined behaviour when the swapping with an inactive thread.
//     this does not include helper threads so try to avoid using this function
//     in diverging control flow where all threads in the quad do not take the same branch.
//     unless you specifically know that the target thread will be active.
bool     quad_read_thread_bool(bool v, uint32_t thread_idx);
half     quad_read_thread_f16(half v, uint32_t thread_idx);
float    quad_read_thread_f32(float v, uint32_t thread_idx);
double   quad_read_thread_f64(double v, uint32_t thread_idx);
int8_t   quad_read_thread_s8(int8_t v, uint32_t thread_idx);
int16_t  quad_read_thread_s16(int16_t v, uint32_t thread_idx);
int32_t  quad_read_thread_s32(int32_t v, uint32_t thread_idx);
int64_t  quad_read_thread_s64(int64_t v, uint32_t thread_idx);
uint8_t  quad_read_thread_u8(uint8_t v, uint32_t thread_idx);
uint16_t quad_read_thread_u16(uint16_t v, uint32_t thread_idx);
uint32_t quad_read_thread_u32(uint32_t v, uint32_t thread_idx);
uint64_t quad_read_thread_u64(uint64_t v, uint32_t thread_idx);
boolx2   quad_read_thread_boolx2(boolx2 v, uint32_t thread_idx);
f16x2    quad_read_thread_f16x2(f16x2 v, uint32_t thread_idx);
f32x2    quad_read_thread_f32x2(f32x2 v, uint32_t thread_idx);
f64x2    quad_read_thread_f64x2(f64x2 v, uint32_t thread_idx);
s8x2     quad_read_thread_s8x2(s8x2 v, uint32_t thread_idx);
s16x2    quad_read_thread_s16x2(s16x2 v, uint32_t thread_idx);
s32x2    quad_read_thread_s32x2(s32x2 v, uint32_t thread_idx);
s64x2    quad_read_thread_s64x2(s64x2 v, uint32_t thread_idx);
u8x2     quad_read_thread_u8x2(u8x2 v, uint32_t thread_idx);
u16x2    quad_read_thread_u16x2(u16x2 v, uint32_t thread_idx);
u32x2    quad_read_thread_u32x2(u32x2 v, uint32_t thread_idx);
u64x2    quad_read_thread_u64x2(u64x2 v, uint32_t thread_idx);
boolx3   quad_read_thread_boolx3(boolx3 v, uint32_t thread_idx);
f16x3    quad_read_thread_f16x3(f16x3 v, uint32_t thread_idx);
f32x3    quad_read_thread_f32x3(f32x3 v, uint32_t thread_idx);
f64x3    quad_read_thread_f64x3(f64x3 v, uint32_t thread_idx);
s8x3     quad_read_thread_s8x3(s8x3 v, uint32_t thread_idx);
s16x3    quad_read_thread_s16x3(s16x3 v, uint32_t thread_idx);
s32x3    quad_read_thread_s32x3(s32x3 v, uint32_t thread_idx);
s64x3    quad_read_thread_s64x3(s64x3 v, uint32_t thread_idx);
u8x3     quad_read_thread_u8x3(u8x3 v, uint32_t thread_idx);
u16x3    quad_read_thread_u16x3(u16x3 v, uint32_t thread_idx);
u32x3    quad_read_thread_u32x3(u32x3 v, uint32_t thread_idx);
u64x3    quad_read_thread_u64x3(u64x3 v, uint32_t thread_idx);
boolx4   quad_read_thread_boolx4(boolx4 v, uint32_t thread_idx);
f16x4    quad_read_thread_f16x4(f16x4 v, uint32_t thread_idx);
f32x4    quad_read_thread_f32x4(f32x4 v, uint32_t thread_idx);
f64x4    quad_read_thread_f64x4(f64x4 v, uint32_t thread_idx);
s8x4     quad_read_thread_s8x4(s8x4 v, uint32_t thread_idx);
s16x4    quad_read_thread_s16x4(s16x4 v, uint32_t thread_idx);
s32x4    quad_read_thread_s32x4(s32x4 v, uint32_t thread_idx);
s64x4    quad_read_thread_s64x4(s64x4 v, uint32_t thread_idx);
u8x4     quad_read_thread_u8x4(u8x4 v, uint32_t thread_idx);
u16x4    quad_read_thread_u16x4(u16x4 v, uint32_t thread_idx);
u32x4    quad_read_thread_u32x4(u32x4 v, uint32_t thread_idx);
u64x4    quad_read_thread_u64x4(u64x4 v, uint32_t thread_idx);

//
// returns true when 'v' from ANY threads in the quad are not equal to '0'
// WARNING:
//     this is undefined behaviour when any thread in your quad is inactive.
//     this does not include helper threads so just avoid using this function
//     in diverging control flow where all threads in the quad do not take the same branch.
bool quad_any_bool(bool v);
bool quad_any_f16(half v);
bool quad_any_f32(float v);
bool quad_any_f64(double v);
bool quad_any_s8(int8_t v);
bool quad_any_s16(int16_t v);
bool quad_any_s32(int32_t v);
bool quad_any_s64(int64_t v);
bool quad_any_u8(uint8_t v);
bool quad_any_u16(uint16_t v);
bool quad_any_u32(uint32_t v);
bool quad_any_u64(uint64_t v);
bool quad_any_boolx2(boolx2 v);
bool quad_any_f16x2(f16x2 v);
bool quad_any_f32x2(f32x2 v);
bool quad_any_f64x2(f64x2 v);
bool quad_any_s8x2(s8x2 v);
bool quad_any_s16x2(s16x2 v);
bool quad_any_s32x2(s32x2 v);
bool quad_any_s64x2(s64x2 v);
bool quad_any_u8x2(u8x2 v);
bool quad_any_u16x2(u16x2 v);
bool quad_any_u32x2(u32x2 v);
bool quad_any_u64x2(u64x2 v);
bool quad_any_boolx3(boolx3 v);
bool quad_any_f16x3(f16x3 v);
bool quad_any_f32x3(f32x3 v);
bool quad_any_f64x3(f64x3 v);
bool quad_any_s8x3(s8x3 v);
bool quad_any_s16x3(s16x3 v);
bool quad_any_s32x3(s32x3 v);
bool quad_any_s64x3(s64x3 v);
bool quad_any_u8x3(u8x3 v);
bool quad_any_u16x3(u16x3 v);
bool quad_any_u32x3(u32x3 v);
bool quad_any_u64x3(u64x3 v);
bool quad_any_boolx4(boolx4 v);
bool quad_any_f16x4(f16x4 v);
bool quad_any_f32x4(f32x4 v);
bool quad_any_f64x4(f64x4 v);
bool quad_any_s8x4(s8x4 v);
bool quad_any_s16x4(s16x4 v);
bool quad_any_s32x4(s32x4 v);
bool quad_any_s64x4(s64x4 v);
bool quad_any_u8x4(u8x4 v);
bool quad_any_u16x4(u16x4 v);
bool quad_any_u32x4(u32x4 v);
bool quad_any_u64x4(u64x4 v);

//
// returns true when 'v' from ALL threads in the quad are not equal to '0'
// WARNING:
//     this is undefined behaviour when any thread in your quad is inactive.
//     this does not include helper threads so just avoid using this function
//     in diverging control flow where all threads in the quad do not take the same branch.
bool quad_all_bool(bool v);
bool quad_all_f16(half v);
bool quad_all_f32(float v);
bool quad_all_f64(double v);
bool quad_all_s8(int8_t v);
bool quad_all_s16(int16_t v);
bool quad_all_s32(int32_t v);
bool quad_all_s64(int64_t v);
bool quad_all_u8(uint8_t v);
bool quad_all_u16(uint16_t v);
bool quad_all_u32(uint32_t v);
bool quad_all_u64(uint64_t v);
bool quad_all_boolx2(boolx2 v);
bool quad_all_f16x2(f16x2 v);
bool quad_all_f32x2(f32x2 v);
bool quad_all_f64x2(f64x2 v);
bool quad_all_s8x2(s8x2 v);
bool quad_all_s16x2(s16x2 v);
bool quad_all_s32x2(s32x2 v);
bool quad_all_s64x2(s64x2 v);
bool quad_all_u8x2(u8x2 v);
bool quad_all_u16x2(u16x2 v);
bool quad_all_u32x2(u32x2 v);
bool quad_all_u64x2(u64x2 v);
bool quad_all_boolx3(boolx3 v);
bool quad_all_f16x3(f16x3 v);
bool quad_all_f32x3(f32x3 v);
bool quad_all_f64x3(f64x3 v);
bool quad_all_s8x3(s8x3 v);
bool quad_all_s16x3(s16x3 v);
bool quad_all_s32x3(s32x3 v);
bool quad_all_s64x3(s64x3 v);
bool quad_all_u8x3(u8x3 v);
bool quad_all_u16x3(u16x3 v);
bool quad_all_u32x3(u32x3 v);
bool quad_all_u64x3(u64x3 v);
bool quad_all_boolx4(boolx4 v);
bool quad_all_f16x4(f16x4 v);
bool quad_all_f32x4(f32x4 v);
bool quad_all_f64x4(f64x4 v);
bool quad_all_s8x4(s8x4 v);
bool quad_all_s16x4(s16x4 v);
bool quad_all_s32x4(s32x4 v);
bool quad_all_s64x4(s64x4 v);
bool quad_all_u8x4(u8x4 v);
bool quad_all_u16x4(u16x4 v);
bool quad_all_u32x4(u32x4 v);
bool quad_all_u64x4(u64x4 v);

//
// returns true when 'v' from ANY active threads in the wave are not equal to '0'.
// for inactive threads these will simply not be contributed to the result.
// unlike quad_any_*, this also works on a wave that has inactive threads
bool wave_active_any_bool(bool v);
bool wave_active_any_f16(half v);
bool wave_active_any_f32(float v);
bool wave_active_any_f64(double v);
bool wave_active_any_s8(int8_t v);
bool wave_active_any_s16(int16_t v);
bool wave_active_any_s32(int32_t v);
bool wave_active_any_s64(int64_t v);
bool wave_active_any_u8(uint8_t v);
bool wave_active_any_u16(uint16_t v);
bool wave_active_any_u32(uint32_t v);
bool wave_active_any_u64(uint64_t v);
bool wave_active_any_boolx2(boolx2 v);
bool wave_active_any_f16x2(f16x2 v);
bool wave_active_any_f32x2(f32x2 v);
bool wave_active_any_f64x2(f64x2 v);
bool wave_active_any_s8x2(s8x2 v);
bool wave_active_any_s16x2(s16x2 v);
bool wave_active_any_s32x2(s32x2 v);
bool wave_active_any_s64x2(s64x2 v);
bool wave_active_any_u8x2(u8x2 v);
bool wave_active_any_u16x2(u16x2 v);
bool wave_active_any_u32x2(u32x2 v);
bool wave_active_any_u64x2(u64x2 v);
bool wave_active_any_boolx3(boolx3 v);
bool wave_active_any_f16x3(f16x3 v);
bool wave_active_any_f32x3(f32x3 v);
bool wave_active_any_f64x3(f64x3 v);
bool wave_active_any_s8x3(s8x3 v);
bool wave_active_any_s16x3(s16x3 v);
bool wave_active_any_s32x3(s32x3 v);
bool wave_active_any_s64x3(s64x3 v);
bool wave_active_any_u8x3(u8x3 v);
bool wave_active_any_u16x3(u16x3 v);
bool wave_active_any_u32x3(u32x3 v);
bool wave_active_any_u64x3(u64x3 v);
bool wave_active_any_boolx4(boolx4 v);
bool wave_active_any_f16x4(f16x4 v);
bool wave_active_any_f32x4(f32x4 v);
bool wave_active_any_f64x4(f64x4 v);
bool wave_active_any_s8x4(s8x4 v);
bool wave_active_any_s16x4(s16x4 v);
bool wave_active_any_s32x4(s32x4 v);
bool wave_active_any_s64x4(s64x4 v);
bool wave_active_any_u8x4(u8x4 v);
bool wave_active_any_u16x4(u16x4 v);
bool wave_active_any_u32x4(u32x4 v);
bool wave_active_any_u64x4(u64x4 v);

//
// returns true when 'v' from ALL active threads in the wave are not equal to '0'
// for inactive threads these will simply not be contributed to the result.
// unlike quad_all_*, this also works on a wave that has inactive threads
bool wave_active_all_bool(bool v);
bool wave_active_all_f16(half v);
bool wave_active_all_f32(float v);
bool wave_active_all_f64(double v);
bool wave_active_all_s8(int8_t v);
bool wave_active_all_s16(int16_t v);
bool wave_active_all_s32(int32_t v);
bool wave_active_all_s64(int64_t v);
bool wave_active_all_u8(uint8_t v);
bool wave_active_all_u16(uint16_t v);
bool wave_active_all_u32(uint32_t v);
bool wave_active_all_u64(uint64_t v);
bool wave_active_all_boolx2(boolx2 v);
bool wave_active_all_f16x2(f16x2 v);
bool wave_active_all_f32x2(f32x2 v);
bool wave_active_all_f64x2(f64x2 v);
bool wave_active_all_s8x2(s8x2 v);
bool wave_active_all_s16x2(s16x2 v);
bool wave_active_all_s32x2(s32x2 v);
bool wave_active_all_s64x2(s64x2 v);
bool wave_active_all_u8x2(u8x2 v);
bool wave_active_all_u16x2(u16x2 v);
bool wave_active_all_u32x2(u32x2 v);
bool wave_active_all_u64x2(u64x2 v);
bool wave_active_all_boolx3(boolx3 v);
bool wave_active_all_f16x3(f16x3 v);
bool wave_active_all_f32x3(f32x3 v);
bool wave_active_all_f64x3(f64x3 v);
bool wave_active_all_s8x3(s8x3 v);
bool wave_active_all_s16x3(s16x3 v);
bool wave_active_all_s32x3(s32x3 v);
bool wave_active_all_s64x3(s64x3 v);
bool wave_active_all_u8x3(u8x3 v);
bool wave_active_all_u16x3(u16x3 v);
bool wave_active_all_u32x3(u32x3 v);
bool wave_active_all_u64x3(u64x3 v);
bool wave_active_all_boolx4(boolx4 v);
bool wave_active_all_f16x4(f16x4 v);
bool wave_active_all_f32x4(f32x4 v);
bool wave_active_all_f64x4(f64x4 v);
bool wave_active_all_s8x4(s8x4 v);
bool wave_active_all_s16x4(s16x4 v);
bool wave_active_all_s32x4(s32x4 v);
bool wave_active_all_s64x4(s64x4 v);
bool wave_active_all_u8x4(u8x4 v);
bool wave_active_all_u16x4(u16x4 v);
bool wave_active_all_u32x4(u32x4 v);
bool wave_active_all_u64x4(u64x4 v);

//
// read 'v' from a thread at 'thread_idx' in the wave
// WARNING:
//     this is undefined behaviour when 'thread_idx' is >= sv->threads_count or when reading from an inactive thread.
//     this does not include helper threads so try to avoid using this function
//     in diverging control flow where all threads in the wave do not take the same branch.
//     unless you specifically know that the target thread will be active.
bool     wave_read_thread_bool(bool v, uint32_t thread_idx);
half     wave_read_thread_f16(half v, uint32_t thread_idx);
float    wave_read_thread_f32(float v, uint32_t thread_idx);
double   wave_read_thread_f64(double v, uint32_t thread_idx);
int8_t   wave_read_thread_s8(int8_t v, uint32_t thread_idx);
int16_t  wave_read_thread_s16(int16_t v, uint32_t thread_idx);
int32_t  wave_read_thread_s32(int32_t v, uint32_t thread_idx);
int64_t  wave_read_thread_s64(int64_t v, uint32_t thread_idx);
uint8_t  wave_read_thread_u8(uint8_t v, uint32_t thread_idx);
uint16_t wave_read_thread_u16(uint16_t v, uint32_t thread_idx);
uint32_t wave_read_thread_u32(uint32_t v, uint32_t thread_idx);
uint64_t wave_read_thread_u64(uint64_t v, uint32_t thread_idx);
boolx2   wave_read_thread_boolx2(boolx2 v, uint32_t thread_idx);
f16x2    wave_read_thread_f16x2(f16x2 v, uint32_t thread_idx);
f32x2    wave_read_thread_f32x2(f32x2 v, uint32_t thread_idx);
f64x2    wave_read_thread_f64x2(f64x2 v, uint32_t thread_idx);
s8x2     wave_read_thread_s8x2(s8x2 v, uint32_t thread_idx);
s16x2    wave_read_thread_s16x2(s16x2 v, uint32_t thread_idx);
s32x2    wave_read_thread_s32x2(s32x2 v, uint32_t thread_idx);
s64x2    wave_read_thread_s64x2(s64x2 v, uint32_t thread_idx);
u8x2     wave_read_thread_u8x2(u8x2 v, uint32_t thread_idx);
u16x2    wave_read_thread_u16x2(u16x2 v, uint32_t thread_idx);
u32x2    wave_read_thread_u32x2(u32x2 v, uint32_t thread_idx);
u64x2    wave_read_thread_u64x2(u64x2 v, uint32_t thread_idx);
boolx3   wave_read_thread_boolx3(boolx3 v, uint32_t thread_idx);
f16x3    wave_read_thread_f16x3(f16x3 v, uint32_t thread_idx);
f32x3    wave_read_thread_f32x3(f32x3 v, uint32_t thread_idx);
f64x3    wave_read_thread_f64x3(f64x3 v, uint32_t thread_idx);
s8x3     wave_read_thread_s8x3(s8x3 v, uint32_t thread_idx);
s16x3    wave_read_thread_s16x3(s16x3 v, uint32_t thread_idx);
s32x3    wave_read_thread_s32x3(s32x3 v, uint32_t thread_idx);
s64x3    wave_read_thread_s64x3(s64x3 v, uint32_t thread_idx);
u8x3     wave_read_thread_u8x3(u8x3 v, uint32_t thread_idx);
u16x3    wave_read_thread_u16x3(u16x3 v, uint32_t thread_idx);
u32x3    wave_read_thread_u32x3(u32x3 v, uint32_t thread_idx);
u64x3    wave_read_thread_u64x3(u64x3 v, uint32_t thread_idx);
boolx4   wave_read_thread_boolx4(boolx4 v, uint32_t thread_idx);
f16x4    wave_read_thread_f16x4(f16x4 v, uint32_t thread_idx);
f32x4    wave_read_thread_f32x4(f32x4 v, uint32_t thread_idx);
f64x4    wave_read_thread_f64x4(f64x4 v, uint32_t thread_idx);
s8x4     wave_read_thread_s8x4(s8x4 v, uint32_t thread_idx);
s16x4    wave_read_thread_s16x4(s16x4 v, uint32_t thread_idx);
s32x4    wave_read_thread_s32x4(s32x4 v, uint32_t thread_idx);
s64x4    wave_read_thread_s64x4(s64x4 v, uint32_t thread_idx);
u8x4     wave_read_thread_u8x4(u8x4 v, uint32_t thread_idx);
u16x4    wave_read_thread_u16x4(u16x4 v, uint32_t thread_idx);
u32x4    wave_read_thread_u32x4(u32x4 v, uint32_t thread_idx);
u64x4    wave_read_thread_u64x4(u64x4 v, uint32_t thread_idx);

//
// returns true when 'v' from ALL active threads in the wave are equal to the same value
// for inactive threads these will simply not be contributed to the result.
bool wave_active_all_equal_bool(bool v);
bool wave_active_all_equal_f16(half v);
bool wave_active_all_equal_f32(float v);
bool wave_active_all_equal_f64(double v);
bool wave_active_all_equal_s8(int8_t v);
bool wave_active_all_equal_s16(int16_t v);
bool wave_active_all_equal_s32(int32_t v);
bool wave_active_all_equal_s64(int64_t v);
bool wave_active_all_equal_u8(uint8_t v);
bool wave_active_all_equal_u16(uint16_t v);
bool wave_active_all_equal_u32(uint32_t v);
bool wave_active_all_equal_u64(uint64_t v);
bool wave_active_all_equal_boolx2(boolx2 v);
bool wave_active_all_equal_f16x2(f16x2 v);
bool wave_active_all_equal_f32x2(f32x2 v);
bool wave_active_all_equal_f64x2(f64x2 v);
bool wave_active_all_equal_s8x2(s8x2 v);
bool wave_active_all_equal_s16x2(s16x2 v);
bool wave_active_all_equal_s32x2(s32x2 v);
bool wave_active_all_equal_s64x2(s64x2 v);
bool wave_active_all_equal_u8x2(u8x2 v);
bool wave_active_all_equal_u16x2(u16x2 v);
bool wave_active_all_equal_u32x2(u32x2 v);
bool wave_active_all_equal_u64x2(u64x2 v);
bool wave_active_all_equal_boolx3(boolx3 v);
bool wave_active_all_equal_f16x3(f16x3 v);
bool wave_active_all_equal_f32x3(f32x3 v);
bool wave_active_all_equal_f64x3(f64x3 v);
bool wave_active_all_equal_s8x3(s8x3 v);
bool wave_active_all_equal_s16x3(s16x3 v);
bool wave_active_all_equal_s32x3(s32x3 v);
bool wave_active_all_equal_s64x3(s64x3 v);
bool wave_active_all_equal_u8x3(u8x3 v);
bool wave_active_all_equal_u16x3(u16x3 v);
bool wave_active_all_equal_u32x3(u32x3 v);
bool wave_active_all_equal_u64x3(u64x3 v);
bool wave_active_all_equal_boolx4(boolx4 v);
bool wave_active_all_equal_f16x4(f16x4 v);
bool wave_active_all_equal_f32x4(f32x4 v);
bool wave_active_all_equal_f64x4(f64x4 v);
bool wave_active_all_equal_s8x4(s8x4 v);
bool wave_active_all_equal_s16x4(s16x4 v);
bool wave_active_all_equal_s32x4(s32x4 v);
bool wave_active_all_equal_s64x4(s64x4 v);
bool wave_active_all_equal_u8x4(u8x4 v);
bool wave_active_all_equal_u16x4(u16x4 v);
bool wave_active_all_equal_u32x4(u32x4 v);
bool wave_active_all_equal_u64x4(u64x4 v);

//
// returns the minimum value of 'v' from ALL active threads in the wave.
// for inactive threads these will simply not be contributed to the result.
half     wave_active_min_f16(half v);
float    wave_active_min_f32(float v);
double   wave_active_min_f64(double v);
int8_t   wave_active_min_s8(int8_t v);
int16_t  wave_active_min_s16(int16_t v);
int32_t  wave_active_min_s32(int32_t v);
int64_t  wave_active_min_s64(int64_t v);
uint8_t  wave_active_min_u8(uint8_t v);
uint16_t wave_active_min_u16(uint16_t v);
uint32_t wave_active_min_u32(uint32_t v);
uint64_t wave_active_min_u64(uint64_t v);
f16x2    wave_active_min_f16x2(f16x2 v);
f32x2    wave_active_min_f32x2(f32x2 v);
f64x2    wave_active_min_f64x2(f64x2 v);
s8x2     wave_active_min_s8x2(s8x2 v);
s16x2    wave_active_min_s16x2(s16x2 v);
s32x2    wave_active_min_s32x2(s32x2 v);
s64x2    wave_active_min_s64x2(s64x2 v);
u8x2     wave_active_min_u8x2(u8x2 v);
u16x2    wave_active_min_u16x2(u16x2 v);
u32x2    wave_active_min_u32x2(u32x2 v);
u64x2    wave_active_min_u64x2(u64x2 v);
f16x3    wave_active_min_f16x3(f16x3 v);
f32x3    wave_active_min_f32x3(f32x3 v);
f64x3    wave_active_min_f64x3(f64x3 v);
s8x3     wave_active_min_s8x3(s8x3 v);
s16x3    wave_active_min_s16x3(s16x3 v);
s32x3    wave_active_min_s32x3(s32x3 v);
s64x3    wave_active_min_s64x3(s64x3 v);
u8x3     wave_active_min_u8x3(u8x3 v);
u16x3    wave_active_min_u16x3(u16x3 v);
u32x3    wave_active_min_u32x3(u32x3 v);
u64x3    wave_active_min_u64x3(u64x3 v);
f16x4    wave_active_min_f16x4(f16x4 v);
f32x4    wave_active_min_f32x4(f32x4 v);
f64x4    wave_active_min_f64x4(f64x4 v);
s8x4     wave_active_min_s8x4(s8x4 v);
s16x4    wave_active_min_s16x4(s16x4 v);
s32x4    wave_active_min_s32x4(s32x4 v);
s64x4    wave_active_min_s64x4(s64x4 v);
u8x4     wave_active_min_u8x4(u8x4 v);
u16x4    wave_active_min_u16x4(u16x4 v);
u32x4    wave_active_min_u32x4(u32x4 v);
u64x4    wave_active_min_u64x4(u64x4 v);

//
// returns the maximum value of 'v' from ALL active threads in the wave.
// for inactive threads these will simply not be contributed to the result.
half     wave_active_max_f16(half v);
float    wave_active_max_f32(float v);
double   wave_active_max_f64(double v);
int8_t   wave_active_max_s8(int8_t v);
int16_t  wave_active_max_s16(int16_t v);
int32_t  wave_active_max_s32(int32_t v);
int64_t  wave_active_max_s64(int64_t v);
uint8_t  wave_active_max_u8(uint8_t v);
uint16_t wave_active_max_u16(uint16_t v);
uint32_t wave_active_max_u32(uint32_t v);
uint64_t wave_active_max_u64(uint64_t v);
f16x2    wave_active_max_f16x2(f16x2 v);
f32x2    wave_active_max_f32x2(f32x2 v);
f64x2    wave_active_max_f64x2(f64x2 v);
s8x2     wave_active_max_s8x2(s8x2 v);
s16x2    wave_active_max_s16x2(s16x2 v);
s32x2    wave_active_max_s32x2(s32x2 v);
s64x2    wave_active_max_s64x2(s64x2 v);
u8x2     wave_active_max_u8x2(u8x2 v);
u16x2    wave_active_max_u16x2(u16x2 v);
u32x2    wave_active_max_u32x2(u32x2 v);
u64x2    wave_active_max_u64x2(u64x2 v);
f16x3    wave_active_max_f16x3(f16x3 v);
f32x3    wave_active_max_f32x3(f32x3 v);
f64x3    wave_active_max_f64x3(f64x3 v);
s8x3     wave_active_max_s8x3(s8x3 v);
s16x3    wave_active_max_s16x3(s16x3 v);
s32x3    wave_active_max_s32x3(s32x3 v);
s64x3    wave_active_max_s64x3(s64x3 v);
u8x3     wave_active_max_u8x3(u8x3 v);
u16x3    wave_active_max_u16x3(u16x3 v);
u32x3    wave_active_max_u32x3(u32x3 v);
u64x3    wave_active_max_u64x3(u64x3 v);
f16x4    wave_active_max_f16x4(f16x4 v);
f32x4    wave_active_max_f32x4(f32x4 v);
f64x4    wave_active_max_f64x4(f64x4 v);
s8x4     wave_active_max_s8x4(s8x4 v);
s16x4    wave_active_max_s16x4(s16x4 v);
s32x4    wave_active_max_s32x4(s32x4 v);
s64x4    wave_active_max_s64x4(s64x4 v);
u8x4     wave_active_max_u8x4(u8x4 v);
u16x4    wave_active_max_u16x4(u16x4 v);
u32x4    wave_active_max_u32x4(u32x4 v);
u64x4    wave_active_max_u64x4(u64x4 v);

//
// returns the sum of 'v' from ALL active threads in the wave.
// for inactive threads these will simply not be contributed to the result.
half     wave_active_sum_f16(half v);
float    wave_active_sum_f32(float v);
double   wave_active_sum_f64(double v);
int8_t   wave_active_sum_s8(int8_t v);
int16_t  wave_active_sum_s16(int16_t v);
int32_t  wave_active_sum_s32(int32_t v);
int64_t  wave_active_sum_s64(int64_t v);
uint8_t  wave_active_sum_u8(uint8_t v);
uint16_t wave_active_sum_u16(uint16_t v);
uint32_t wave_active_sum_u32(uint32_t v);
uint64_t wave_active_sum_u64(uint64_t v);
f16x2    wave_active_sum_f16x2(f16x2 v);
f32x2    wave_active_sum_f32x2(f32x2 v);
f64x2    wave_active_sum_f64x2(f64x2 v);
s8x2     wave_active_sum_s8x2(s8x2 v);
s16x2    wave_active_sum_s16x2(s16x2 v);
s32x2    wave_active_sum_s32x2(s32x2 v);
s64x2    wave_active_sum_s64x2(s64x2 v);
u8x2     wave_active_sum_u8x2(u8x2 v);
u16x2    wave_active_sum_u16x2(u16x2 v);
u32x2    wave_active_sum_u32x2(u32x2 v);
u64x2    wave_active_sum_u64x2(u64x2 v);
f16x3    wave_active_sum_f16x3(f16x3 v);
f32x3    wave_active_sum_f32x3(f32x3 v);
f64x3    wave_active_sum_f64x3(f64x3 v);
s8x3     wave_active_sum_s8x3(s8x3 v);
s16x3    wave_active_sum_s16x3(s16x3 v);
s32x3    wave_active_sum_s32x3(s32x3 v);
s64x3    wave_active_sum_s64x3(s64x3 v);
u8x3     wave_active_sum_u8x3(u8x3 v);
u16x3    wave_active_sum_u16x3(u16x3 v);
u32x3    wave_active_sum_u32x3(u32x3 v);
u64x3    wave_active_sum_u64x3(u64x3 v);
f16x4    wave_active_sum_f16x4(f16x4 v);
f32x4    wave_active_sum_f32x4(f32x4 v);
f64x4    wave_active_sum_f64x4(f64x4 v);
s8x4     wave_active_sum_s8x4(s8x4 v);
s16x4    wave_active_sum_s16x4(s16x4 v);
s32x4    wave_active_sum_s32x4(s32x4 v);
s64x4    wave_active_sum_s64x4(s64x4 v);
u8x4     wave_active_sum_u8x4(u8x4 v);
u16x4    wave_active_sum_u16x4(u16x4 v);
u32x4    wave_active_sum_u32x4(u32x4 v);
u64x4    wave_active_sum_u64x4(u64x4 v);

//
// returns the sum of 'v' from ALL active the previous threads in the wave.
// for inactive threads these will simply not be contributed to the result.
half     wave_active_prefix_sum_f16(half v);
float    wave_active_prefix_sum_f32(float v);
double   wave_active_prefix_sum_f64(double v);
int8_t   wave_active_prefix_sum_s8(int8_t v);
int16_t  wave_active_prefix_sum_s16(int16_t v);
int32_t  wave_active_prefix_sum_s32(int32_t v);
int64_t  wave_active_prefix_sum_s64(int64_t v);
uint8_t  wave_active_prefix_sum_u8(uint8_t v);
uint16_t wave_active_prefix_sum_u16(uint16_t v);
uint32_t wave_active_prefix_sum_u32(uint32_t v);
uint64_t wave_active_prefix_sum_u64(uint64_t v);
f16x2    wave_active_prefix_sum_f16x2(f16x2 v);
f32x2    wave_active_prefix_sum_f32x2(f32x2 v);
f64x2    wave_active_prefix_sum_f64x2(f64x2 v);
s8x2     wave_active_prefix_sum_s8x2(s8x2 v);
s16x2    wave_active_prefix_sum_s16x2(s16x2 v);
s32x2    wave_active_prefix_sum_s32x2(s32x2 v);
s64x2    wave_active_prefix_sum_s64x2(s64x2 v);
u8x2     wave_active_prefix_sum_u8x2(u8x2 v);
u16x2    wave_active_prefix_sum_u16x2(u16x2 v);
u32x2    wave_active_prefix_sum_u32x2(u32x2 v);
u64x2    wave_active_prefix_sum_u64x2(u64x2 v);
f16x3    wave_active_prefix_sum_f16x3(f16x3 v);
f32x3    wave_active_prefix_sum_f32x3(f32x3 v);
f64x3    wave_active_prefix_sum_f64x3(f64x3 v);
s8x3     wave_active_prefix_sum_s8x3(s8x3 v);
s16x3    wave_active_prefix_sum_s16x3(s16x3 v);
s32x3    wave_active_prefix_sum_s32x3(s32x3 v);
s64x3    wave_active_prefix_sum_s64x3(s64x3 v);
u8x3     wave_active_prefix_sum_u8x3(u8x3 v);
u16x3    wave_active_prefix_sum_u16x3(u16x3 v);
u32x3    wave_active_prefix_sum_u32x3(u32x3 v);
u64x3    wave_active_prefix_sum_u64x3(u64x3 v);
f16x4    wave_active_prefix_sum_f16x4(f16x4 v);
f32x4    wave_active_prefix_sum_f32x4(f32x4 v);
f64x4    wave_active_prefix_sum_f64x4(f64x4 v);
s8x4     wave_active_prefix_sum_s8x4(s8x4 v);
s16x4    wave_active_prefix_sum_s16x4(s16x4 v);
s32x4    wave_active_prefix_sum_s32x4(s32x4 v);
s64x4    wave_active_prefix_sum_s64x4(s64x4 v);
u8x4     wave_active_prefix_sum_u8x4(u8x4 v);
u16x4    wave_active_prefix_sum_u16x4(u16x4 v);
u32x4    wave_active_prefix_sum_u32x4(u32x4 v);
u64x4    wave_active_prefix_sum_u64x4(u64x4 v);

//
// returns the product of 'v' from ALL active threads in the wave.
// for inactive threads these will simply not be contributed to the result.
half     wave_active_product_f16(half v);
float    wave_active_product_f32(float v);
double   wave_active_product_f64(double v);
int8_t   wave_active_product_s8(int8_t v);
int16_t  wave_active_product_s16(int16_t v);
int32_t  wave_active_product_s32(int32_t v);
int64_t  wave_active_product_s64(int64_t v);
uint8_t  wave_active_product_u8(uint8_t v);
uint16_t wave_active_product_u16(uint16_t v);
uint32_t wave_active_product_u32(uint32_t v);
uint64_t wave_active_product_u64(uint64_t v);
f16x2    wave_active_product_f16x2(f16x2 v);
f32x2    wave_active_product_f32x2(f32x2 v);
f64x2    wave_active_product_f64x2(f64x2 v);
s8x2     wave_active_product_s8x2(s8x2 v);
s16x2    wave_active_product_s16x2(s16x2 v);
s32x2    wave_active_product_s32x2(s32x2 v);
s64x2    wave_active_product_s64x2(s64x2 v);
u8x2     wave_active_product_u8x2(u8x2 v);
u16x2    wave_active_product_u16x2(u16x2 v);
u32x2    wave_active_product_u32x2(u32x2 v);
u64x2    wave_active_product_u64x2(u64x2 v);
f16x3    wave_active_product_f16x3(f16x3 v);
f32x3    wave_active_product_f32x3(f32x3 v);
f64x3    wave_active_product_f64x3(f64x3 v);
s8x3     wave_active_product_s8x3(s8x3 v);
s16x3    wave_active_product_s16x3(s16x3 v);
s32x3    wave_active_product_s32x3(s32x3 v);
s64x3    wave_active_product_s64x3(s64x3 v);
u8x3     wave_active_product_u8x3(u8x3 v);
u16x3    wave_active_product_u16x3(u16x3 v);
u32x3    wave_active_product_u32x3(u32x3 v);
u64x3    wave_active_product_u64x3(u64x3 v);
f16x4    wave_active_product_f16x4(f16x4 v);
f32x4    wave_active_product_f32x4(f32x4 v);
f64x4    wave_active_product_f64x4(f64x4 v);
s8x4     wave_active_product_s8x4(s8x4 v);
s16x4    wave_active_product_s16x4(s16x4 v);
s32x4    wave_active_product_s32x4(s32x4 v);
s64x4    wave_active_product_s64x4(s64x4 v);
u8x4     wave_active_product_u8x4(u8x4 v);
u16x4    wave_active_product_u16x4(u16x4 v);
u32x4    wave_active_product_u32x4(u32x4 v);
u64x4    wave_active_product_u64x4(u64x4 v);

//
// returns the product of 'v' from ALL active the previous threads in the wave.
// for inactive threads these will simply not be contributed to the result.
half     wave_active_prefix_product_f16(half v);
float    wave_active_prefix_product_f32(float v);
double   wave_active_prefix_product_f64(double v);
int8_t   wave_active_prefix_product_s8(int8_t v);
int16_t  wave_active_prefix_product_s16(int16_t v);
int32_t  wave_active_prefix_product_s32(int32_t v);
int64_t  wave_active_prefix_product_s64(int64_t v);
uint8_t  wave_active_prefix_product_u8(uint8_t v);
uint16_t wave_active_prefix_product_u16(uint16_t v);
uint32_t wave_active_prefix_product_u32(uint32_t v);
uint64_t wave_active_prefix_product_u64(uint64_t v);
f16x2    wave_active_prefix_product_f16x2(f16x2 v);
f32x2    wave_active_prefix_product_f32x2(f32x2 v);
f64x2    wave_active_prefix_product_f64x2(f64x2 v);
s8x2     wave_active_prefix_product_s8x2(s8x2 v);
s16x2    wave_active_prefix_product_s16x2(s16x2 v);
s32x2    wave_active_prefix_product_s32x2(s32x2 v);
s64x2    wave_active_prefix_product_s64x2(s64x2 v);
u8x2     wave_active_prefix_product_u8x2(u8x2 v);
u16x2    wave_active_prefix_product_u16x2(u16x2 v);
u32x2    wave_active_prefix_product_u32x2(u32x2 v);
u64x2    wave_active_prefix_product_u64x2(u64x2 v);
f16x3    wave_active_prefix_product_f16x3(f16x3 v);
f32x3    wave_active_prefix_product_f32x3(f32x3 v);
f64x3    wave_active_prefix_product_f64x3(f64x3 v);
s8x3     wave_active_prefix_product_s8x3(s8x3 v);
s16x3    wave_active_prefix_product_s16x3(s16x3 v);
s32x3    wave_active_prefix_product_s32x3(s32x3 v);
s64x3    wave_active_prefix_product_s64x3(s64x3 v);
u8x3     wave_active_prefix_product_u8x3(u8x3 v);
u16x3    wave_active_prefix_product_u16x3(u16x3 v);
u32x3    wave_active_prefix_product_u32x3(u32x3 v);
u64x3    wave_active_prefix_product_u64x3(u64x3 v);
f16x4    wave_active_prefix_product_f16x4(f16x4 v);
f32x4    wave_active_prefix_product_f32x4(f32x4 v);
f64x4    wave_active_prefix_product_f64x4(f64x4 v);
s8x4     wave_active_prefix_product_s8x4(s8x4 v);
s16x4    wave_active_prefix_product_s16x4(s16x4 v);
s32x4    wave_active_prefix_product_s32x4(s32x4 v);
s64x4    wave_active_prefix_product_s64x4(s64x4 v);
u8x4     wave_active_prefix_product_u8x4(u8x4 v);
u16x4    wave_active_prefix_product_u16x4(u16x4 v);
u32x4    wave_active_prefix_product_u32x4(u32x4 v);
u64x4    wave_active_prefix_product_u64x4(u64x4 v);

//
// returns a count of the number of 'v' that are 'true' in ALL active threads in the wave.
// for inactive threads these will simply not be contributed to the result.
uint32_t wave_active_count_bits(bool v);

//
// returns a count of the number of 'v' that are 'true' in ALL of the previous active threads in the wave.
// for inactive threads these will simply not be contributed to the result.
uint32_t wave_active_prefix_count_bits(bool v);

//
// returns the bitwise AND operation of 'v' from ALL active threads in the wave.
// for inactive threads these will simply not be contributed to the result.
int8_t   wave_active_bit_and_s8(int8_t v);
int16_t  wave_active_bit_and_s16(int16_t v);
int32_t  wave_active_bit_and_s32(int32_t v);
int64_t  wave_active_bit_and_s64(int64_t v);
uint8_t  wave_active_bit_and_u8(uint8_t v);
uint16_t wave_active_bit_and_u16(uint16_t v);
uint32_t wave_active_bit_and_u32(uint32_t v);
uint64_t wave_active_bit_and_u64(uint64_t v);
s8x2     wave_active_bit_and_s8x2(s8x2 v);
s16x2    wave_active_bit_and_s16x2(s16x2 v);
s32x2    wave_active_bit_and_s32x2(s32x2 v);
s64x2    wave_active_bit_and_s64x2(s64x2 v);
u8x2     wave_active_bit_and_u8x2(u8x2 v);
u16x2    wave_active_bit_and_u16x2(u16x2 v);
u32x2    wave_active_bit_and_u32x2(u32x2 v);
u64x2    wave_active_bit_and_u64x2(u64x2 v);
s8x3     wave_active_bit_and_s8x3(s8x3 v);
s16x3    wave_active_bit_and_s16x3(s16x3 v);
s32x3    wave_active_bit_and_s32x3(s32x3 v);
s64x3    wave_active_bit_and_s64x3(s64x3 v);
u8x3     wave_active_bit_and_u8x3(u8x3 v);
u16x3    wave_active_bit_and_u16x3(u16x3 v);
u32x3    wave_active_bit_and_u32x3(u32x3 v);
u64x3    wave_active_bit_and_u64x3(u64x3 v);
s8x4     wave_active_bit_and_s8x4(s8x4 v);
s16x4    wave_active_bit_and_s16x4(s16x4 v);
s32x4    wave_active_bit_and_s32x4(s32x4 v);
s64x4    wave_active_bit_and_s64x4(s64x4 v);
u8x4     wave_active_bit_and_u8x4(u8x4 v);
u16x4    wave_active_bit_and_u16x4(u16x4 v);
u32x4    wave_active_bit_and_u32x4(u32x4 v);
u64x4    wave_active_bit_and_u64x4(u64x4 v);

//
// returns the bitwise OR operation of 'v' from ALL active threads in the wave.
// for inactive threads these will simply not be contributed to the result.
int8_t   wave_active_bit_or_s8(int8_t v);
int16_t  wave_active_bit_or_s16(int16_t v);
int32_t  wave_active_bit_or_s32(int32_t v);
int64_t  wave_active_bit_or_s64(int64_t v);
uint8_t  wave_active_bit_or_u8(uint8_t v);
uint16_t wave_active_bit_or_u16(uint16_t v);
uint32_t wave_active_bit_or_u32(uint32_t v);
uint64_t wave_active_bit_or_u64(uint64_t v);
s8x2     wave_active_bit_or_s8x2(s8x2 v);
s16x2    wave_active_bit_or_s16x2(s16x2 v);
s32x2    wave_active_bit_or_s32x2(s32x2 v);
s64x2    wave_active_bit_or_s64x2(s64x2 v);
u8x2     wave_active_bit_or_u8x2(u8x2 v);
u16x2    wave_active_bit_or_u16x2(u16x2 v);
u32x2    wave_active_bit_or_u32x2(u32x2 v);
u64x2    wave_active_bit_or_u64x2(u64x2 v);
s8x3     wave_active_bit_or_s8x3(s8x3 v);
s16x3    wave_active_bit_or_s16x3(s16x3 v);
s32x3    wave_active_bit_or_s32x3(s32x3 v);
s64x3    wave_active_bit_or_s64x3(s64x3 v);
u8x3     wave_active_bit_or_u8x3(u8x3 v);
u16x3    wave_active_bit_or_u16x3(u16x3 v);
u32x3    wave_active_bit_or_u32x3(u32x3 v);
u64x3    wave_active_bit_or_u64x3(u64x3 v);
s8x4     wave_active_bit_or_s8x4(s8x4 v);
s16x4    wave_active_bit_or_s16x4(s16x4 v);
s32x4    wave_active_bit_or_s32x4(s32x4 v);
s64x4    wave_active_bit_or_s64x4(s64x4 v);
u8x4     wave_active_bit_or_u8x4(u8x4 v);
u16x4    wave_active_bit_or_u16x4(u16x4 v);
u32x4    wave_active_bit_or_u32x4(u32x4 v);
u64x4    wave_active_bit_or_u64x4(u64x4 v);

//
// returns the bitwise XOR operation of 'v' from ALL active threads in the wave.
// for inactive threads these will simply not be contributed to the result.
int8_t   wave_active_bit_xor_s8(int8_t v);
int16_t  wave_active_bit_xor_s16(int16_t v);
int32_t  wave_active_bit_xor_s32(int32_t v);
int64_t  wave_active_bit_xor_s64(int64_t v);
uint8_t  wave_active_bit_xor_u8(uint8_t v);
uint16_t wave_active_bit_xor_u16(uint16_t v);
uint32_t wave_active_bit_xor_u32(uint32_t v);
uint64_t wave_active_bit_xor_u64(uint64_t v);
s8x2     wave_active_bit_xor_s8x2(s8x2 v);
s16x2    wave_active_bit_xor_s16x2(s16x2 v);
s32x2    wave_active_bit_xor_s32x2(s32x2 v);
s64x2    wave_active_bit_xor_s64x2(s64x2 v);
u8x2     wave_active_bit_xor_u8x2(u8x2 v);
u16x2    wave_active_bit_xor_u16x2(u16x2 v);
u32x2    wave_active_bit_xor_u32x2(u32x2 v);
u64x2    wave_active_bit_xor_u64x2(u64x2 v);
s8x3     wave_active_bit_xor_s8x3(s8x3 v);
s16x3    wave_active_bit_xor_s16x3(s16x3 v);
s32x3    wave_active_bit_xor_s32x3(s32x3 v);
s64x3    wave_active_bit_xor_s64x3(s64x3 v);
u8x3     wave_active_bit_xor_u8x3(u8x3 v);
u16x3    wave_active_bit_xor_u16x3(u16x3 v);
u32x3    wave_active_bit_xor_u32x3(u32x3 v);
u64x3    wave_active_bit_xor_u64x3(u64x3 v);
s8x4     wave_active_bit_xor_s8x4(s8x4 v);
s16x4    wave_active_bit_xor_s16x4(s16x4 v);
s32x4    wave_active_bit_xor_s32x4(s32x4 v);
s64x4    wave_active_bit_xor_s64x4(s64x4 v);
u8x4     wave_active_bit_xor_u8x4(u8x4 v);
u16x4    wave_active_bit_xor_u16x4(u16x4 v);
u32x4    wave_active_bit_xor_u32x4(u32x4 v);
u64x4    wave_active_bit_xor_u64x4(u64x4 v);

#endif // _HCC_WAVE_INTRINSICS_H_

