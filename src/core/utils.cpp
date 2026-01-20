#ifdef _WIN32
    #include <winsock.h>
#else
    #include <arpa/inet.h>
#endif
#include <cstring>
#include <chrono>
#include <random>

#include "core/utils.hpp"

namespace snow {
    void serialize_f32(u8* buffer, f32 value) {
        u32 as_int;
        memcpy(&as_int, &value, sizeof(f32));
        as_int = htonl(as_int);
        memcpy(buffer, &as_int, sizeof(u32));
    }

    f32 deserialize_float(const u8* buffer) {
        u32 as_int;
        memcpy(&as_int, buffer, sizeof(u32));

        as_int = ntohl(as_int);

        f32 value;
        memcpy(&value, &as_int, sizeof(f32));
        return value;
    }

    void serialize_u64(u8* buffer, u64 value) {
        *buffer = htonll(value);
    }

    u64 deserialize_u64(const u8* buffer) {
        return reinterpret_cast<u64>(buffer);
    }

    /**
     * Returns the local Unix timestamp.
     * NOTE: This should NEVER be used for measuring
     * time between two computers. Two computers running this
     * function at the same time are not guaranteed to return
     * the same value. Only use for local measurements!
     */
    u64 get_local_timestamp() {
        const auto clock = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            clock.time_since_epoch()).count();
    }

    /**
     * Generates a random UUIDv4 string.
     */
    std::string generate_uuid() {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        std::uniform_int_distribution<int> dist(0, 15);

        const char* v = "0123456789abcdef";
        const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

        std::string res;
        for (int i = 0; i < 16; i++) {
            if (dash[i]) res += "-";
            res += v[dist(rng)];
            res += v[dist(rng)];
        }
        return res;
    }

    /**
     * 64-bit host to network
     */
    u64 htonll(u64 value) {
        #if __BIG_ENDIAN__
            return value;
        #else
            return (u64)htonl((u32)(value >> 32)) | (u64)htonl((u32)(value & 0xFFFFFFFF)) << 32;
        #endif
    }

    /**
     * 64-bit network to host
     */
    u64 ntohll(u64 value) {
        #if __BIG_ENDIAN__
            return value;
        #else
            return (u64)ntohl((u32)(value >> 32)) | (u64)ntohl((u32)(value & 0xFFFFFFFF)) << 32;
        #endif
    }
}
