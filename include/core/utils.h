#pragma once

#include <string>
#include <stdint.h>
#include <cstdarg>
#include <cstdio>

namespace snow {
    // 36 characters plus null terminator
    constexpr uint32_t _UUID_SIZE = 37;

    void serialize_float(uint8_t* buffer, float value);
    float deserialize_float(const uint8_t* buffer);
    void serialize_uint64_t(uint8_t* buffer, uint64_t value);
    uint64_t deserialize_uint64_t(const uint8_t* buffer);

    uint64_t htonll(uint64_t value);
    uint64_t ntohll(uint64_t value);

    uint64_t get_local_timestamp();
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
