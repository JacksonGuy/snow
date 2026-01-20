#pragma once

#include <mutex>
#include <vector>
#include <functional>

#include "enet/enet.h"

#include "core/utils.hpp"
#include "net/packet.hpp"

namespace snow {
    constexpr size_t _CHANNEL_LIMIT = 2;
    constexpr size_t _PEER_LIMIT = 512;

    typedef struct {
        char uuid[UUID_SIZE];
        ENetPeer* peer;
    } ClientInfo;

    typedef struct {
        ENetEvent event;
        Packet packet;
    } Message;

    class Server {
        public:
            u16 port;
            u16 tick_rate;  // Ticks per second

            Server(u16 port = 8000);
            ~Server();
            void init();
            void start(std::function<void(Server&)> user_loop);

        private:
            ENetHost* host;

            // Clients attempting to connect
            std::mutex new_connections_mtx;
            std::vector<ENetEvent> new_connections;

            // Currently connected clients
            std::mutex clients_mtx;
            std::vector<ClientInfo> clients;

            // Messsages from clients to server
            std::mutex messages_mtx;
            std::vector<Message> messages;

            void receive_messages();
            void disconnect_client(ENetEvent* event);
    };
}
