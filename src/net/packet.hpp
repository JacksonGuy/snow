#pragma once

#include <stddef.h>
#include <memory>

#include "enet/enet.h"

#include "core/utils.hpp"
#include "core/serializable.hpp"

namespace snow {
    class Packet : public Serializable {
        public:
            u64 type;
            u64 timestamp;
            u64 priority;
            u64 size;
            std::unique_ptr<u8> data;

            Packet();
            Packet(const Packet& packet);
            Packet(Packet&& packet) noexcept;
            Packet& operator=(const Packet& packet);
            Packet& operator=(Packet&& packet) noexcept;
            Packet(ENetEvent* event);
            ~Packet();

            u64 get_size() const noexcept;
            u8* Serialize() const override;
            bool Deserialize(const u8* buffer) override;
    };
}
