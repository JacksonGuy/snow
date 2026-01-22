#include "net/client.hpp"
#include "core/utils.hpp"
#include "enet/enet.h"
#include "enet/protocol.h"

namespace snow {
    Client::Client() {
        this->connection = nullptr;
        this->server = nullptr;
        this->uuid = Packet::default_uuid;
    }

    Client::~Client() {

    }

    bool Client::connect_to_server(const char* ip, u16 port) {
        if (enet_initialize() != 0) {
            debug_error("Failed to initialize ENet.");
            return false;
        }

        ENetEvent event;
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

        // Create local ENet host
        this->connection = enet_host_create(
            nullptr,        // Address
            1,              // Peer count
            0,              // Channel Limit
            0,              // Incoming bandwidth
            0               // Outgoing bandwidth
        );
        if (this->connection == nullptr) {
            debug_error("Failed to create local connection.");
            return false;
        }

        // Connect to server
        enet_address_set_host(&address, ip);
        this->server = enet_host_connect(
            this->connection,                       // Host
            &address,                               // Server address
            ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT,    // Channel count
            0                                       // Data
        );
        if (this->server == nullptr) {
            debug_error("Failed to connect to server.");
            return false;
        }

        // Listen for connect packet with 10sec timeout
        if (
            (enet_host_service(this->connection, &event, 10000) < 0) ||
            event.type != ENET_EVENT_TYPE_CONNECT
        ) {
            debug_error("Connection failed: Did not receive connect packet");
            enet_peer_reset(this->server);
            return false;
        }

        // Get UUID from server
        while (1) {
            if (enet_host_service(this->connection, &event, 5000) > 0) {
                Packet packet(&event);
                this->uuid = packet.uuid;
            }
        }

        return true;
    }
}
