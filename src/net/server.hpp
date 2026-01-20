#pragma once

#include <mutex>
#include <vector>
#include <queue>
#include <functional>

#include "enet/enet.h"

#include "core/utils.hpp"
#include "net/packet.hpp"

namespace snow {
    constexpr size_t _CHANNEL_LIMIT = 2;
    constexpr size_t _PEER_LIMIT = 512;
    constexpr size_t _PING_HANDSHAKE_AMOUNT = 10;

    typedef struct {
        char uuid[_UUID_SIZE];
        ENetPeer* peer;

        u64 total_ping_count;
        u64 total_ping_sum;
        u64 ping_average;
    } ClientInfo;

    typedef struct {
        ENetEvent event;
        Packet packet;
    } Message;

    class Server {
        public:
            u16 port;
            u16 tick_rate;  // Ticks per second
            u32 max_clients;

            Server(u16 port = 8000, u32 max_clients = 32);
            ~Server();
            void init();
            void start(
                std::function<void(Server&)> user_loop,
                std::function<void(Server&)> connect_callback,
                std::function<void(Server&)> disconnect_callback
            );

        private:
            ENetHost* host;

            // Clients attempting to connect
            std::mutex new_connections_mtx;
            std::queue<ENetEvent> new_connections;

            // Client in the handshake process
            std::mutex handshake_clients_mtx;
            std::vector<ClientInfo> handshake_clients;

            // Currently connected clients
            std::mutex clients_mtx;
            std::vector<ClientInfo> clients;

            // Messsages from clients to server
            std::mutex messages_mtx;
            std::vector<Message> messages;

            void poll_events();
            void handle_new_connections();
            void main_loop(std::function<void(Server&)> user_loop);
            void disconnect_client(ENetEvent& event);
            void begin_client_handshake(ENetEvent& event);
            void finalize_client_handshake(ClientInfo& client);
    };
}
