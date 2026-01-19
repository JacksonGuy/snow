#ifdef _WIN32
    #include <winsock.h>
#else
    #include <arpa/inet.h>
#endif
#include <cstring>

#include "utils.hpp"

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

    u64 htonll(u64 value) {
        #if __BIG_ENDIAN__
            return value;
        #else
            return (u64)htonl((u32)(value >> 32)) | (u64)htonl((u32)(value & 0xFFFFFFFF)) << 32;
        #endif
    }

    u64 ntohll(u64 value) {
        #if __BIG_ENDIAN__
            return value;
        #else
            return (u64)ntohl((u32)(value >> 32)) | (u64)ntohl((u32)(value & 0xFFFFFFFF)) << 32;
        #endif
    }
}
