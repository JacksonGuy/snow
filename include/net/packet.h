#pragma once

#include <stddef.h>
#include <memory>

#include "enet/enet.h"

#include "core/utils.h"

namespace snow {
    class Packet {
        public:
            static constexpr char default_uuid[] = "00000000-0000-0000-0000-000000000000";

            char uuid[_UUID_SIZE];
            size_t size;
            std::unique_ptr<uint8_t[]> data;

            Packet();
            Packet(const Packet& packet);
            Packet(Packet&& packet) noexcept;
            Packet& operator=(const Packet& packet);
            Packet& operator=(Packet&& packet) noexcept;
            Packet(ENetEvent* event);
            ~Packet();

            size_t get_size() const noexcept;
            uint8_t* serialize() const;
            bool deserialize(const uint8_t* buffer);
    };
}
