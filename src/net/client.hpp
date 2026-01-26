#pragma once

#include <functional>

#include "enet/enet.h"

#include "core/utils.hpp"
#include "net/packet.hpp"

namespace snow {
    class Client {
        public:
            Client();
            ~Client();
            bool connect_to_server(const char* ip, u16 port);
            void poll_events(std::function<void(ENetEvent&)> user_callback);
            void send_packet(const Packet& packet, bool reliable, u8 channel);
            const std::string& get_uuid() const;

        private:
            ENetHost* connection;
            ENetPeer* server;
            std::string uuid;
    };
}
