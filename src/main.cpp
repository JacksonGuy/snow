#include <iostream>
#include <thread>

#include <cassert>

#include "enet/enet.h"

#define PORT 8080

void server_test() {
    ENetHost* host = nullptr;
    ENetPeer* peer = nullptr;

    const ENetAddress addr = { ENET_HOST_ANY, PORT };
    host = enet_host_create(&addr, 1, 1, 0, 0);
    assert(host != nullptr);

    std::cout << "Server started." << std::endl;

    bool done = false;
    while (!done) {
        ENetEvent event;
        const int service_result = enet_host_service(host, &event, 1);
        if (service_result <= 0) continue;

        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
            {
                std::cout << "[INFO] Client Connected." << std::endl;
                peer = event.peer;

                break;
            }

            case ENET_EVENT_TYPE_RECEIVE:
            {
                std::cout << "[MESSAGE] " << (const char*)event.packet->data << std::endl;
                enet_packet_destroy(event.packet);
                enet_peer_disconnect(peer, 0);

                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT:
            {
                std::cout << "[INFO] Client Disconnected." << std::endl;
                done = true;

                break;
            }

            case ENET_EVENT_TYPE_NONE:
            {
                break;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Server starting..." << std::endl;

    server_test();

    std::cout << "Server shut down." << std::endl;

    return 0;
}
