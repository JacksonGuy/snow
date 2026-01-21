#pragma once

#include <mutex>
#include <unordered_map>
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
        std::string uuid;
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
                std::function<bool(Server&, ENetEvent&)> connect_callback = nullptr,
                std::function<void(Server&, ENetEvent&)> disconnect_callback = nullptr
            );

        private:
            ENetHost* host;

            // User function pointers
            std::function<void(Server&)> _user_loop;
            std::function<bool(Server&, ENetEvent&)> _user_connect_callback;
            std::function<void(Server&, ENetEvent&)> _user_disconnect_callback;

            // All clients lookup
            std::mutex client_lookup_mtx;
            std::unordered_map<char*, ENetPeer*> client_lookup;

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

            // Main server methods
            void poll_events();
            void handle_new_connections();
            void main_loop();
            void disconnect_client(ENetEvent& event);
            void begin_client_handshake(ENetEvent& event);
            void finalize_client_handshake(ClientInfo& client);
    };
}
