#pragma once

#include <stddef.h>
#include <memory>

#include "enet/enet.h"

#include "core/utils.hpp"
#include "core/serializable.hpp"

namespace snow {
    class Packet : public Serializable {
        public:
            static constexpr char default_uuid[] = "00000000-0000-0000-0000-000000000000";

            char uuid[_UUID_SIZE];
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

    void send_packet(ENetPeer* to, const Packet& packet, bool reliable, int channel);
}
