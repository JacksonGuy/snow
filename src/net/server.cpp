#include <chrono>
#include <cstdlib>
#include <thread>
#include <functional>

#include "enet/enet.h"

#include "net/server.hpp"
#include "core/utils.hpp"

namespace snow {
    Server::Server(u16 port) {
        this->port = port;
        this->tick_rate = 20;
        this->host = nullptr;
    }

    /**
     * Initialize and start the server instance.
     * Creates an ENet host and creates threads
     * for communication.
     */
    void Server::init() {
        if (enet_initialize() != 0) {
            debug_error("Failed to initialize ENet");
            exit(EXIT_FAILURE);
        }

        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = this->port;
        this->host = enet_host_create(
            &address,
            snow::_PEER_LIMIT,      // Maximum player count
            snow::_CHANNEL_LIMIT,   // Communication channels
            0,                      // Incoming bandwidth
            0                       // Outgoing bandwidth
        );

        // NULL for compatibility
        if (this->host == NULL || this->host == nullptr) {
            debug_error("Failed to create server host");
            exit(EXIT_FAILURE);
        }
    }

    void Server::start(std::function<void(Server&)> user_loop) {
        const i32 tick_time = 1000.0 / this->tick_rate;
        u64 current_time = get_local_timestamp();

        while (1) {
            u64 last_tick = get_local_timestamp() - current_time;

            if (last_tick < tick_time) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(tick_time - last_tick)
                );
            }
            else if (last_tick > tick_time) {
                u64 behind = last_tick - tick_time;
                debug_warn("Server ran %llu ms behind", behind);
            }

            current_time = get_local_timestamp();

            this->receive_messages();
        }
    }

    void Server::receive_messages() {
        const i32 tick_time = 1000.0 / this->tick_rate;
        ENetEvent event;

        while (1) {
            while (enet_host_service(this->host, &event, tick_time) > 0)
            {
                switch (event.type)
                {
                    case ENET_EVENT_TYPE_CONNECT:
                    {
                        debug_log("New connection.");

                        std::lock_guard<std::mutex> lock(this->new_connections_mtx);
                        this->new_connections.push_back(event);

                        break;
                    }

                    case ENET_EVENT_TYPE_RECEIVE:
                    {
                        debug_log("Message received.");

                        std::lock_guard<std::mutex> lock(this->messages_mtx);

                        Packet packet(&event);
                        Message msg = {
                            .event = event,
                            .packet = packet
                        };
                        this->messages.push_back(msg);

                        break;
                    }

                    case ENET_EVENT_TYPE_DISCONNECT:
                    {
                        debug_log("Client disconnected.");
                        disconnect_client(&event);

                        break;
                    }

                    case ENET_EVENT_TYPE_NONE:
                    {
                        break;
                    }
                }

                enet_packet_destroy(event.packet);
            }
        }
    }

    void Server::disconnect_client(ENetEvent* event) {

    }
}
