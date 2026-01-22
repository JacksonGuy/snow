#pragma once

#include "enet/enet.h"

#include "core/utils.hpp"
#include "net/packet.hpp"

namespace snow {
    class Client {
        public:
            Client();
            ~Client();
            bool connect_to_server(const char* ip, u16 port);

        private:
            ENetHost* connection;
            ENetPeer* server;
            std::string uuid;
    };
}
