#pragma once

#include <sstream>
#include <string>
#include <stdint.h>
#include <cstdarg>
#include <cstdio>
#include <thread>

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
    // 36 characters plus null terminator
    constexpr u32 UUID_SIZE = 37;

    void serialize_f32(u8* buffer, f32 value);
    f32 deserialize_f32(const u8* buffer);
    void serialize_u64(u8* buffer, u64 value);
    u64 deserialize_u64(const u8* buffer);

    u64 htonll(u64 value);
    u64 ntohll(u64 value);

    u64 get_local_timestamp();
    std::string generate_uuid();

    inline void debug_log_impl(
        const char* level,
        const char* file,
        int line,
        const char* text,
        ...
    ) {
        std::fprintf(stderr, "[%s][%s:%d] ", level, file, line);

        va_list args;
        va_start(args, text);
        std::vfprintf(stderr, text, args);
        va_end(args);

        std::fprintf(stderr, "\n");
    }

    #ifdef DEBUG_BUILD
        #define debug_log(text, ...) \
            snow::debug_log_impl("INFO", __FILE__, __LINE__, text, ##__VA_ARGS__)

        #define debug_warn(text, ...) \
            snow::debug_log_impl("WARN", __FILE__, __LINE__, text, ##__VA_ARGS__)

        #define debug_error(text, ...) \
            snow::debug_log_impl("ERROR", __FILE__, __LINE__, text, ##__VA_ARGS__)
    #else
        #define debug_log(...) ((void)0)
        #define debug_warn(...) ((void)0)
        #define debug_error(...) ((void)0)
    #endif
}
