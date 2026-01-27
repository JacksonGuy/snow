#ifdef _WIN32
    #include <winsock.h>
#else
    #include <arpa/inet.h>
#endif
#include <cstring>
#include <chrono>
#include <random>

#include "core/utils.h"

namespace snow {
    void serialize_float(uint8_t* buffer, float value) {
        uint32_t as_int;
        memcpy(&as_int, &value, sizeof(float));
        as_int = htonl(as_int);
        memcpy(buffer, &as_int, sizeof(uint32_t));
    }

    float deserialize_float(const uint8_t* buffer) {
        uint32_t as_int;
        memcpy(&as_int, buffer, sizeof(uint32_t));

        as_int = ntohl(as_int);

        float value;
        memcpy(&value, &as_int, sizeof(float));
        return value;
    }

    void serialize_uint64_t(uint8_t* buffer, uint64_t value) {
        *(uint64_t*)buffer = htonll(value);
    }

    uint64_t deserialize_uint64_t(const uint8_t* buffer) {
        return ntohll(*(uint64_t*)buffer);
    }

    /**
     * Returns the local Unix timestamp.
     * NOTE: This should NEVER be used for measuring
     * time between two computers. Two computers running this
     * function at the same time are not guaranteed to return
     * the same value. Only use for local measurements!
     */
    uint64_t get_local_timestamp() {
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
    uint64_t htonll(uint64_t value) {
        #if __BIG_ENDIAN__
            return value;
        #else
            return (uint64_t)htonl((uint32_t)(value >> 32)) | (uint64_t)htonl((uint32_t)(value & 0xFFFFFFFF)) << 32;
        #endif
    }

    /**
     * 64-bit network to host
     */
    uint64_t ntohll(uint64_t value) {
        #if __BIG_ENDIAN__
            return value;
        #else
            return (uint64_t)ntohl((uint32_t)(value >> 32)) | (uint64_t)ntohl((uint32_t)(value & 0xFFFFFFFF)) << 32;
        #endif
    }
}
