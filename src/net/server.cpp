#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <functional>
#include <cstring>

#include "enet/enet.h"

#include "net/server.hpp"
#include "core/utils.hpp"
#include "net/packet.hpp"

namespace snow {
    Server::Server(u16 port, u32 max_clients) {
        this->port = port;
        this->tick_rate = 20;
        this->max_clients = max_clients;
        this->host = nullptr;

        this->_user_loop = nullptr;
        this->_user_connect_callback = nullptr;
        this->_user_disconnect_callback = nullptr;
    }

    Server::~Server() {
        debug_log("[SERVER] Stopping server...");

        // Disconnect clients
        for (const ClientInfo& client : this->clients) {
            enet_peer_disconnect(client.peer, 0);
        }

        // Destroy host instance
        enet_host_destroy(this->host);
        this->host = nullptr;
    }

    /**
     * Initialize and start the server instance.
     * Creates an ENet host and creates threads
     * for communication.
     */
    void Server::init() {
        if (enet_initialize() != 0) {
            debug_error("[SERVER] Failed to initialize ENet.");
            exit(EXIT_FAILURE);
        }

        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = this->port;
        this->host = enet_host_create(
            &address,
            this->max_clients,      // Maximum player count
            0,                      // Communication channels
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
        std::function<bool(Server&, ENetEvent&)> connect_callback,
        std::function<void(Server&, ENetEvent&)> disconnect_callback
    ) {
        // Set user functions
        this->_user_loop = user_loop;
        this->_user_connect_callback = connect_callback;
        this->_user_disconnect_callback = disconnect_callback;

        main_loop();
    }

    /*
     * Poll ENet for network events.
     */
    void Server::poll_events() {
        ENetEvent event;

        while (enet_host_service(this->host, &event, 0) > 0)
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                {
                    debug_log("[SERVER] New connection.");

                    bool result = true;
                    if (this->_user_connect_callback != nullptr) {
                        debug_log("[SERVER] Calling user connect callback...");
                        result = this->_user_connect_callback(*this, event);
                    }

                    // Result value for callback determines if
                    // we continue allowing the client to connect.
                    if (result) {
                        debug_log("[SERVER] Handling new connection...");
                        this->handle_new_connection(event);
                    }

                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE:
                {
                    debug_log("[SERVER] Message received.");

                    Packet packet(&event);

                    // Client validation check
                    auto peer_it = this->client_lookup.find(packet.uuid);
                    if (peer_it == this->client_lookup.end()) {
                        debug_error("[SERVER] Client with UUID %s does not exist.", packet.uuid);
                        break;
                    }
                    ENetPeer* peer = this->client_lookup[packet.uuid];
                    if (peer->connectID != event.peer->connectID) {
                        debug_error("[SERVER] Client sent packet with incorrect UUID");
                        break;
                    }

                    Message msg = {
                        .event = event,
                        .packet = packet
                    };
                    this->incoming_messages.push(msg);

                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT:
                {
                    debug_log("[SERVER] Client disconnected.");

                    if (this->_user_disconnect_callback != nullptr) {
                        this->_user_disconnect_callback(*this, event);
                    }
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

    /*
     * Finalize new client connection.
     */
    void Server::handle_new_connection(ENetEvent& event) {
        // Create ClientInfo object for client
        ClientInfo client;
        client.peer = event.peer;
        client.uuid = generate_uuid();

        // Send client their UUID
        Packet uuid_packet;
        strcpy(uuid_packet.uuid, client.uuid.data());
        _send_packet_immediate(uuid_packet, client.peer, true, snow::_CHANNEL_RELIABLE);
        debug_log("[SERVER] UUID sent to client.");

        // Add client to server
        this->client_lookup[client.uuid.data()] = client.peer;
        this->clients.push_back(client);
        // this->client_lookup.insert(std::make_pair(client.uuid.data(), client.peer));
    }

    void Server::main_loop() {
        const i32 tick_time = 1000.0 / this->tick_rate;
        u64 last_tick_timestamp = get_local_timestamp();

        while (1) {
            u64 time_since_last_tick = get_local_timestamp() - last_tick_timestamp;

            if (time_since_last_tick < tick_time) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(tick_time - time_since_last_tick)
                );
            }
            else if (time_since_last_tick > tick_time) {
                u64 behind = time_since_last_tick - tick_time;
                debug_warn("[SERVER] Server ran %llu ms behind", behind);
            }

            last_tick_timestamp = get_local_timestamp();

            debug_log("[SERVER] Polling Events...");
            this->poll_events();

            debug_log("[SERVER] Running user loop...");
            this->_user_loop(*this);

            // Send out all queued packets
            while (!this->outgoing_messages.empty()) {
                QueuePacket& message = this->outgoing_messages.front();

                if (message.dest != nullptr) {
                    _send_packet_immediate(message.packet, message.dest, message.reliable, message.channel);
                }
                else {
                    _broadcast_packet_immediate(message.packet, message.reliable, message.channel);
                }

                this->outgoing_messages.pop();
            }
        }
    }

    /*
     * Removes the client from the server client list.
     */
    void Server::disconnect_client(ENetEvent& event) {
        // Search and remove client
        bool found = false;
        auto it = this->clients.begin();
        while (it != this->clients.end()) {
            if (it->peer->connectID == event.peer->connectID) {
                this->client_lookup.erase(it->uuid.data());
                this->clients.erase(it);
                debug_log("[SERVER] Client successfully disconnected.");
                return;
            }
        }

        if (!found) {
            debug_error("[SERVER] Failed to disconnect client: client doesn't exist");
        }
    }

    void Server::send_packet(const Packet& packet, ENetPeer* dest, bool reliable, u8 channel) {
        this->outgoing_messages.emplace((QueuePacket){
            .packet = std::move(packet),
            .dest = dest,
            .reliable = reliable,
            .channel = channel,
        });
    }

    void Server::broadcast_packet(const Packet& packet, bool reliable, u8 channel) {
        this->outgoing_messages.emplace((QueuePacket){
            .packet = std::move(packet),
            .dest = nullptr,
            .reliable = reliable,
            .channel = channel,
        });
    }

    Message* Server::read_packet() {
        if (this->incoming_messages.empty()) {
            return nullptr;
        }

        this->message_cache = std::move(this->incoming_messages.front());
        this->incoming_messages.pop();
        return &this->message_cache;
    }

    /*
     * Send packet directly to client.
     */
    void Server::_send_packet_immediate(const Packet& packet, ENetPeer* dest, bool reliable, u8 channel) {
        size_t flag = 0;

        if (reliable) {
            flag = ENET_PACKET_FLAG_RELIABLE;
        }
        else {
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }

        u8* bytes = packet.Serialize();
        size_t size = packet.get_size();
        ENetPacket* enet_packet = enet_packet_create(bytes, size, flag);

        enet_peer_send(dest, channel, enet_packet);

        free(bytes);
    }

    /*
     * Broadcast packet to all clients.
     */
    void Server::_broadcast_packet_immediate(const Packet& packet, bool reliable, u8 channel) {
        size_t flag = 0;

        if (reliable) {
            flag = ENET_PACKET_FLAG_RELIABLE;
        }
        else {
            flag = ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }

        u8* bytes = packet.Serialize();
        size_t size = packet.get_size();
        ENetPacket* enet_packet = enet_packet_create(bytes, size, flag);

        for (const ClientInfo& client : this->clients) {
            enet_peer_send(client.peer, channel, enet_packet);
        }

        free(bytes);
    }
}
