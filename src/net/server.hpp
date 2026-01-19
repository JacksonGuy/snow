#pragma once

#include "enet/enet.h"

namespace snow {
    typedef struct {
        ENetPeer* peer;
    } ClientInfo;

    class Server {
        public:

        private:
            ENetHost* host;
    };
}
