#ifndef PW_CTYPES_H
#define PW_CTYPES_H

#include <stdint.h>
#include <stddef.h>

typedef char      byte;
typedef uint8_t   u8;
typedef int32_t   i32;
typedef uint32_t  u32;
typedef int64_t   i64;
typedef uint64_t  u64;
typedef float     f32;
typedef double    f64;
typedef size_t    usize;
typedef ptrdiff_t ssize;
typedef uintptr_t uptr;

#define ARR_SIZE(a)     (ssize)(sizeof(a) / sizeof(*(a)))

#endif // PW_CTYPES_H