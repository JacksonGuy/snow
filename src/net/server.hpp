#pragma once

#include "core/utils.hpp"

#include "enet/enet.h"

namespace snow {
    typedef struct {
        char uuid[UUID_SIZE];
        ENetPeer* peer;
    } ClientInfo;

    class Server {
        public:
            Server();
            ~Server();


        private:
            ENetHost* host;
    };
}
