
//
// waits for the atomic mutations operations in resource memory for all threads across all waves for this shared work to be visible
void memory_barrier_resource(void);

//
// waits for the atomic mutations operations in shared work memory for all threads across all waves for this shared work to be visible
void memory_barrier_shared_work(void);

//
// waits for the atomic mutations operations in resource & shared work memory for all threads across all waves for this shared work to be visible
void memory_barrier_all(void);

//
// waits for the atomic mutations operations in resource memory for all threads across all waves for this shared work to be visible
// and also waits for all threads across all waves in the shared work before continuing
void control_barrier_resource(void);

//
// waits for the atomic mutations operations in shared work memory all threads across all waves for this shared work to be visible.
// and also waits for all threads across all waves in the shared work before continuing
void control_barrier_shared_work(void);

//
// waits for the atomic mutations operations in resource & shared work memory all threads across all waves for this shared work to be visible.
// and also waits for all threads across all waves in the shared work before continuing
void control_barrier_all(void);

//
// atomically load the value at pointer 'ptr'
half atomic_load_f16(half const* ptr);
float atomic_load_f32(float const* ptr);
double atomic_load_f64(double const* ptr);
int8_t atomic_load_s8(int8_t const* ptr);
int16_t atomic_load_s16(int16_t const* ptr);
int32_t atomic_load_s32(int32_t const* ptr);
int64_t atomic_load_s64(int64_t const* ptr);
uint8_t atomic_load_u8(uint8_t const* ptr);
uint16_t atomic_load_u16(uint16_t const* ptr);
uint32_t atomic_load_u32(uint32_t const* ptr);
uint64_t atomic_load_u64(uint64_t const* ptr);

//
// atomically store the value 'v' at pointer 'ptr'
void atomic_store_f16(half* ptr, half v);
void atomic_store_f32(float* ptr, float v);
void atomic_store_f64(double* ptr, double v);
void atomic_store_s8(int8_t* ptr, int8_t v);
void atomic_store_s16(int16_t* ptr, int16_t v);
void atomic_store_s32(int32_t* ptr, int32_t v);
void atomic_store_s64(int64_t* ptr, int64_t v);
void atomic_store_u8(uint8_t* ptr, uint8_t v);
void atomic_store_u16(uint16_t* ptr, uint16_t v);
void atomic_store_u32(uint32_t* ptr, uint32_t v);
void atomic_store_u64(uint64_t* ptr, uint64_t v);

//
// atomically store the value 'v' at pointer 'ptr' and the original value at 'ptr' is returned
half atomic_exchange_f16(half* ptr, half v);
float atomic_exchange_f32(float* ptr, float v);
double atomic_exchange_f64(double* ptr, double v);
int8_t atomic_exchange_s8(int8_t* ptr, int8_t v);
int16_t atomic_exchange_s16(int16_t* ptr, int16_t v);
int32_t atomic_exchange_s32(int32_t* ptr, int32_t v);
int64_t atomic_exchange_s64(int64_t* ptr, int64_t v);
uint8_t atomic_exchange_u8(uint8_t* ptr, uint8_t v);
uint16_t atomic_exchange_u16(uint16_t* ptr, uint16_t v);
uint32_t atomic_exchange_u32(uint32_t* ptr, uint32_t v);
uint64_t atomic_exchange_u64(uint64_t* ptr, uint64_t v);

//
// atomically store the value 'v' at pointer 'ptr' if the value at 'ptr' is equal to 'cond'.
// the original value at 'ptr' is returned always
int8_t atomic_compare_exchange_s8(int8_t* ptr, int8_t cond, int8_t v);
int16_t atomic_compare_exchange_s16(int16_t* ptr, int16_t cond, int16_t v);
int32_t atomic_compare_exchange_s32(int32_t* ptr, int32_t cond, int32_t v);
int64_t atomic_compare_exchange_s64(int64_t* ptr, int64_t cond, int64_t v);
uint8_t atomic_compare_exchange_u8(uint8_t* ptr, uint8_t cond, uint8_t v);
uint16_t atomic_compare_exchange_u16(uint16_t* ptr, uint16_t cond, uint16_t v);
uint32_t atomic_compare_exchange_u32(uint32_t* ptr, uint32_t cond, uint32_t v);
uint64_t atomic_compare_exchange_u64(uint64_t* ptr, uint64_t cond, uint64_t v);

//
// atomically add the value 'v' to the value at pointer 'ptr'. the original value at 'ptr' is returned
int8_t atomic_add_s8(int8_t* ptr, int8_t v);
int16_t atomic_add_s16(int16_t* ptr, int16_t v);
int32_t atomic_add_s32(int32_t* ptr, int32_t v);
int64_t atomic_add_s64(int64_t* ptr, int64_t v);
uint8_t atomic_add_u8(uint8_t* ptr, uint8_t v);
uint16_t atomic_add_u16(uint16_t* ptr, uint16_t v);
uint32_t atomic_add_u32(uint32_t* ptr, uint32_t v);
uint64_t atomic_add_u64(uint64_t* ptr, uint64_t v);

//
// atomically subtract the value 'v' to the value at pointer 'ptr'. the original value at 'ptr' is returned
int8_t atomic_sub_s8(int8_t* ptr, int8_t v);
int16_t atomic_sub_s16(int16_t* ptr, int16_t v);
int32_t atomic_sub_s32(int32_t* ptr, int32_t v);
int64_t atomic_sub_s64(int64_t* ptr, int64_t v);
uint8_t atomic_sub_u8(uint8_t* ptr, uint8_t v);
uint16_t atomic_sub_u16(uint16_t* ptr, uint16_t v);
uint32_t atomic_sub_u32(uint32_t* ptr, uint32_t v);
uint64_t atomic_sub_u64(uint64_t* ptr, uint64_t v);

//
// atomically min the value 'v' to the value at pointer 'ptr'. the original value at 'ptr' is returned
int8_t atomic_min_s8(int8_t* ptr, int8_t v);
int16_t atomic_min_s16(int16_t* ptr, int16_t v);
int32_t atomic_min_s32(int32_t* ptr, int32_t v);
int64_t atomic_min_s64(int64_t* ptr, int64_t v);
uint8_t atomic_min_u8(uint8_t* ptr, uint8_t v);
uint16_t atomic_min_u16(uint16_t* ptr, uint16_t v);
uint32_t atomic_min_u32(uint32_t* ptr, uint32_t v);
uint64_t atomic_min_u64(uint64_t* ptr, uint64_t v);

//
// atomically max the value 'v' to the value at pointer 'ptr'. the original value at 'ptr' is returned
int8_t atomic_max_s8(int8_t* ptr, int8_t v);
int16_t atomic_max_s16(int16_t* ptr, int16_t v);
int32_t atomic_max_s32(int32_t* ptr, int32_t v);
int64_t atomic_max_s64(int64_t* ptr, int64_t v);
uint8_t atomic_max_u8(uint8_t* ptr, uint8_t v);
uint16_t atomic_max_u16(uint16_t* ptr, uint16_t v);
uint32_t atomic_max_u32(uint32_t* ptr, uint32_t v);
uint64_t atomic_max_u64(uint64_t* ptr, uint64_t v);

//
// atomically bitwise AND the value 'v' to the value at pointer 'ptr'. the original value at 'ptr' is returned
int8_t atomic_bit_and_s8(int8_t* ptr, int8_t v);
int16_t atomic_bit_and_s16(int16_t* ptr, int16_t v);
int32_t atomic_bit_and_s32(int32_t* ptr, int32_t v);
int64_t atomic_bit_and_s64(int64_t* ptr, int64_t v);
uint8_t atomic_bit_and_u8(uint8_t* ptr, uint8_t v);
uint16_t atomic_bit_and_u16(uint16_t* ptr, uint16_t v);
uint32_t atomic_bit_and_u32(uint32_t* ptr, uint32_t v);
uint64_t atomic_bit_and_u64(uint64_t* ptr, uint64_t v);

//
// atomically bitwise OR the value 'v' to the value at pointer 'ptr'. the original value at 'ptr' is returned
int8_t atomic_bit_or_s8(int8_t* ptr, int8_t v);
int16_t atomic_bit_or_s16(int16_t* ptr, int16_t v);
int32_t atomic_bit_or_s32(int32_t* ptr, int32_t v);
int64_t atomic_bit_or_s64(int64_t* ptr, int64_t v);
uint8_t atomic_bit_or_u8(uint8_t* ptr, uint8_t v);
uint16_t atomic_bit_or_u16(uint16_t* ptr, uint16_t v);
uint32_t atomic_bit_or_u32(uint32_t* ptr, uint32_t v);
uint64_t atomic_bit_or_u64(uint64_t* ptr, uint64_t v);

//
// atomically bit XOR the value 'v' to the value at pointer 'ptr'. the original value at 'ptr' is returned
int8_t atomic_bit_xor_s8(int8_t* ptr, int8_t v);
int16_t atomic_bit_xor_s16(int16_t* ptr, int16_t v);
int32_t atomic_bit_xor_s32(int32_t* ptr, int32_t v);
int64_t atomic_bit_xor_s64(int64_t* ptr, int64_t v);
uint8_t atomic_bit_xor_u8(uint8_t* ptr, uint8_t v);
uint16_t atomic_bit_xor_u16(uint16_t* ptr, uint16_t v);
uint32_t atomic_bit_xor_u32(uint32_t* ptr, uint32_t v);
uint64_t atomic_bit_xor_u64(uint64_t* ptr, uint64_t v);

