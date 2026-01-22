#pragma once

#include <unordered_map>
#include <vector>
#include <queue>
#include <functional>

#include "enet/enet.h"

#include "core/utils.hpp"
#include "net/packet.hpp"

namespace snow {
    typedef struct {
        std::string uuid;
        ENetPeer* peer;
    } ClientInfo;

    typedef struct {
        ENetEvent event;
        Packet packet;
    } Message;

    typedef struct {
        Packet packet;
        ENetPeer* dest;
        bool reliable;
        u8 channel;
    } QueuePacket;

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
            void send_packet(const Packet& packet, ENetPeer* dest, bool reliable, u8 channel);
            void broadcast_packet(const Packet& packet, bool reliable, u8 channel);

        private:
            static const u8 _CHANNEL_RELIABLE = 0;
            static const u8 _CHANNEL_UNRELIABLE = 1;

            ENetHost* host;

            // User function pointers
            std::function<void(Server&)> _user_loop;
            std::function<bool(Server&, ENetEvent&)> _user_connect_callback;
            std::function<void(Server&, ENetEvent&)> _user_disconnect_callback;

            // Client lookup
            std::unordered_map<char*, ENetPeer*> client_lookup;

            // Client list
            std::vector<ClientInfo> clients;

            // Message between client and server
            // NOTE: using std::queue here might become an
            // issue in the future.
            std::queue<Message> incoming_messages;
            std::queue<QueuePacket> outgoing_messages;

            void poll_events();
            void handle_new_connection(ENetEvent& event);
            void main_loop();
            void disconnect_client(ENetEvent& event);

            void _send_packet_immediate(const Packet& packet, ENetPeer* dest, bool reliable, u8 channel);
            void _broadcast_packet_immediate(const Packet& packet, bool reliable, u8 channel);
    };
}
