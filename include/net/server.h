#pragma once

#include <unordered_map>
#include <vector>
#include <queue>
#include <functional>

#include "enet/enet.h"

#include "core/utils.h"
#include "net/packet.h"

namespace snow {
    const uint8_t _CHANNEL_RELIABLE = 0;
    const uint8_t _CHANNEL_UNRELIABLE = 1;

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
        uint8_t channel;
    } QueuePacket;

    class Server {
        public:
            uint16_t port;
            uint16_t tick_rate;  // Ticks per second
            uint32_t max_clients;

            Server(uint16_t port = 8000, uint32_t max_clients = 32);
            ~Server();
            void init();
            void start(
                std::function<void(Server&)> user_loop,
                std::function<bool(Server&, ENetEvent&)> connect_callback = nullptr,
                std::function<void(Server&, ENetEvent&)> disconnect_callback = nullptr
            );
            void send_packet(const Packet& packet, ENetPeer* dest, bool reliable, uint8_t channel);
            void broadcast_packet(const Packet& packet, bool reliable, uint8_t channel);
            Message* read_packet();

        private:
            ENetHost* m_host;
            Message m_message_cache;

            // User function pointers
            std::function<void(Server&)> m_user_loop;
            std::function<bool(Server&, ENetEvent&)> m_user_connect_callback;
            std::function<void(Server&, ENetEvent&)> m_user_disconnect_callback;

            // Client lookup
            std::unordered_map<std::string, ENetPeer*> m_client_lookup;

            // Client list
            std::vector<ClientInfo> m_clients;

            // Message between client and server
            // NOTE: using std::queue here might become an
            // issue in the future.
            std::queue<Message> m_incoming_messages;
            std::queue<QueuePacket> m_outgoing_messages;

            void poll_events();
            void handle_new_connection(ENetEvent& event);
            void main_loop();
            void disconnect_client(ENetEvent& event);

            void _send_packet_immediate(const Packet& packet, ENetPeer* dest, bool reliable, uint8_t channel);
            void _broadcast_packet_immediate(const Packet& packet, bool reliable, uint8_t channel);
    };
}
