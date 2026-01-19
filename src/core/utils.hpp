#pragma once

#include <stdint.h>

typedef float       f32;
typedef double      f64;
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;
typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

namespace snow {
    constexpr u32 UUID_SIZE = 37;

    void serialize_f32(u8* buffer, f32 value);
    f32 deserialize_f32(const u8* buffer);
    void serialize_u64(u8* buffer, u64 value);
    u64 deserialize_u64(const u8* buffer);

    u64 htonll(u64 value);
    u64 ntohll(u64 value);
}
