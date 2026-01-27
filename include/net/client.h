#pragma once

#include <functional>

#include "enet/enet.h"

#include "core/utils.h"
#include "net/packet.h"

namespace snow {
    class Client {
        public:
            Client();
            ~Client();
            bool connect_to_server(const char* ip, uint16_t port);
            void poll_events(std::function<void(ENetEvent&)> user_callback);
            void send_packet(const Packet& packet, bool reliable, uint8_t channel);
            const std::string& get_uuid() const;

        private:
            ENetHost* m_connection;
            ENetPeer* m_server;
            std::string m_uuid;
    };
}
