#pragma once

#include <stddef.h>

#include "core/utils.hpp"
#include "core/serializable.hpp"

namespace snow {
    class Packet : public Serializable {
        public:
            u64 type;
            u64 timestamp;
            u64 priority;
            size_t size;
            u8* data;

            Packet();
            Packet(const Packet& packet);
            Packet(Packet&& packet);
            Packet& operator=(const Packet& packet);
            Packet& operator=(Packet&& packet);
            ~Packet();

            u8* Serialize() const override;
            bool Deserialize(u8* data) override;
    };
}
