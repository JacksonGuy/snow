#include <iostream>
#include <cstdlib>

#include "enet/enet.h"

#include "net/client.h"
#include "core/utils.h"

namespace snow {
    Client::Client() {
        this->m_connection = nullptr;
        this->m_server = nullptr;
        this->m_uuid = Packet::default_uuid;
    }

    Client::~Client() {
        enet_host_destroy(this->m_connection);
    }

    bool Client::connect_to_server(const char* ip, uint16_t port) {
        if (enet_initialize() != 0) {
            debug_error("Failed to initialize ENet.");
            return false;
        }

        ENetEvent event;
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

        // Create local ENet host
        this->m_connection = enet_host_create(
            nullptr,        // Address
            1,              // Peer count
            0,              // Channel Limit
            0,              // Incoming bandwidth
            0               // Outgoing bandwidth
        );
        if (this->m_connection == nullptr) {
            debug_error("Failed to create local connection.");
            return false;
        }
        debug_log("[CLIENT] Host Created.");

        // Connect to server
        enet_address_set_host(&address, ip);
        this->m_server = enet_host_connect(
            this->m_connection,                       // Host
            &address,                               // Server address
            ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT,    // Channel count
            0                                       // Data
        );
        if (this->m_server == nullptr) {
            debug_error("Failed to connect to server.");
            return false;
        }
        debug_log("[CLIENT] Connection Established.");

        // Listen for connect packet
        bool connection_packet = false;
        uint8_t connect_attempts = 0;
        while (1) {
            if (connect_attempts >= 6) {
                break;
            }

            if ((enet_host_service(this->m_connection, &event, 5000) > 0)) {
                connection_packet = true;
                break;
            }
        }
        if (!connection_packet) {
            debug_error("Connection failed: Did not receive connect packet");
            enet_peer_reset(this->m_server);
            return false;
        }
        debug_log("[CLIENT] Connection packet received.");

        // Get UUID from server
        debug_log("[CLIENT] Waiting for UUID...");
        bool uuid_packet = false;
        uint8_t uuid_attempts = 0;
        while (1) {
            if (uuid_attempts > 12) {
                break;
            }

            if (enet_host_service(this->m_connection, &event, 5000) > 0) {
                Packet packet(&event);

                // Extra (probably useless) check to ensure
                // we are looking at the correct packet.
                if (packet.size == 0) {
                    this->m_uuid = packet.uuid;
                    uuid_packet = true;
                    break;
                }
            }
        }
        if (!uuid_packet) {
            debug_error("Connection failed: Did not receive UUID from server");
            enet_peer_reset(this->m_server);
            return false;
        }

        debug_log("[CLIENT] UUID Received: %s", this->m_uuid.c_str());

        return true;
    }

    void Client::poll_events(std::function<void(ENetEvent&)> user_callback) {
        ENetEvent event;

        while (enet_host_service(this->m_connection, &event, 0) > 0) {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_RECEIVE:
                {
                    if (event.packet->data == nullptr || event.packet->data == NULL) break;

                    user_callback(event);

                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT:
                {
                    // Destroy ENET objects and force exit
                    enet_peer_reset(this->m_server);
                    enet_host_destroy(this->m_connection);
                    exit(EXIT_SUCCESS);

                    break;
                }

                default:
                {
                    break;
                }
            }
        }
    }

    void Client::send_packet(const Packet& packet, bool reliable, uint8_t channel) {
        size_t flag = 0;

        if (reliable) {
            flag = ENET_PACKET_FLAG_RELIABLE;
        }
        else {
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }

        uint8_t* bytes = packet.serialize();
        size_t size = packet.get_size();
        ENetPacket* enet_packet = enet_packet_create(bytes, size, flag);

        enet_peer_send(this->m_server, channel, enet_packet);

        free(bytes);
    }

    const std::string& Client::get_uuid() const {
        return this->m_uuid;
    }
}
