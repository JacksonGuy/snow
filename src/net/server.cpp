#include <chrono>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <functional>

#include "enet/enet.h"

#include "net/server.hpp"
#include "core/utils.hpp"

namespace snow {
    Server::Server(u16 port, u32 max_clients) {
        this->port = port;
        this->tick_rate = 20;
        this->max_clients = max_clients;
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

    /*
     * Creates main server threads for receiving and responding
     * to client packets.
     */
    void Server::start(
        std::function<void(Server&)> user_loop,
        std::function<void(Server&)> connect_callback,
        std::function<void(Server&)> disconnect_callback
    ) {
        /*
         * t1 : Listen for packets from clients
         * t2 : Perform client handshakes
         * t3 : Main update loop
         */
        std::thread t1(&Server::poll_events, this);
        std::thread t2(&Server::handle_new_connections, this);
        std::thread t3(&Server::main_loop, this, user_loop);
    }

    /*
     * Poll ENet for network events.
     */
    void Server::poll_events() {
        const i32 tick_time = 1000.0 / this->tick_rate;
        ENetEvent event;

        while (1) {
            // timeout = tick_time
            while (enet_host_service(this->host, &event, tick_time) > 0)
            {
                switch (event.type)
                {
                    case ENET_EVENT_TYPE_CONNECT:
                    {
                        debug_log("New connection.");

                        std::lock_guard<std::mutex> lock(this->new_connections_mtx);
                        this->new_connections.push(event);

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

                        disconnect_client(event);

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

    /*
     * Initiate handshake procedures and poll clients
     * currently in the handshake process.
     */
    void Server::handle_new_connections() {
        while (1) {
            // Check for new incomming connections
            {
                std::lock_guard<std::mutex> lock(this->new_connections_mtx);

                if (!this->new_connections.empty()) {
                    // Temporary lock on clients list to obtain
                    // player count estimate.
                    size_t player_count = 0;
                    {
                        std::lock_guard<std::mutex> temp_lock(this->clients_mtx);
                        player_count = this->clients.size();
                    }

                    bool server_full = player_count < this->max_clients;

                    // Empty out new connections queue
                    while (!this->new_connections.empty()) {
                        ENetEvent& event = this->new_connections.front();

                        if (server_full) {
                            debug_warn("Server is full! Disconnecting player.");

                            disconnect_client(event);
                        }
                        else {
                            // Send client their UUID and some ping
                            // packets to get a RTT estimate.
                            begin_client_handshake(event);
                        }

                        this->new_connections.pop();
                    }
                }
            }

            // Handle client currently in handshake process
            {
                std::lock_guard<std::mutex> lock(this->handshake_clients_mtx);

                auto it = this->handshake_clients.begin();
                while (it != this->handshake_clients.end()) {
                    ClientInfo& client = *it;

                    if (client.total_ping_count >= snow::_PING_HANDSHAKE_AMOUNT) {
                        finalize_client_handshake(client);
                        it = this->handshake_clients.erase(it);
                    }
                    else {
                        it++;
                    }
                }
            }

            // Sleep so we aren't holding locks for too long
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void Server::main_loop(std::function<void(Server&)> user_loop) {
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
        }
    }

    // TODO
    void Server::disconnect_client(ENetEvent& event) {
        std::lock_guard<std::mutex> lock(this->clients_mtx);
    }

    // TODO
    void Server::begin_client_handshake(ENetEvent& event) {

    }

    // TODO
    void Server::finalize_client_handshake(ClientInfo& client) {

    }
}
