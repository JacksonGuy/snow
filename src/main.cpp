#include <chrono>
#include <iostream>
#include <cassert>
#include <memory>
#include <thread>
#include <string.h>
#include <stdlib.h>

#include "core/utils.h"
#include "net/server.h"
#include "net/client.h"
#include "net/packet.h"

int main(int argc, char* argv[]) {
    using namespace snow;

    std::thread server_thread([]() {
        Server server(8080);
        server.init();
        server.start([](Server& server) {
            Message* msg = server.read_packet();
            while (msg != nullptr) {
                Packet& packet = msg->packet;
                std::cout << "[SERVER] Received message from " << packet.uuid << std::endl;

                server.broadcast_packet(packet, true, snow::_CHANNEL_RELIABLE);

                msg = server.read_packet();
            }
        });
    });

    std::thread client_thread([]() {
        Client client;
        bool result = client.connect_to_server("127.0.0.1", 8080);
        if (!result) {
            std::cout << "Failed to connect to server. Exiting..." << std::endl;
        }
        else {
            std::string message = "Hello World";

            while (1) {
                debug_log("[CLIENT] Polling events...");
                client.poll_events([](ENetEvent& event) {
                    Packet packet(&event);

                    if (packet.size <= 0) return;

                    char* str = (char*)malloc(packet.size);
                    memcpy(str, packet.data.get(), packet.size);
                    std::cout << "Server: " << str << std::endl;
                });
                debug_log("[CLIENT] Done.");

                // TODO make function for this
                Packet packet;
                strcpy(packet.uuid, client.get_uuid().c_str());
                packet.size = message.size() + 1;
                packet.data = std::make_unique<uint8_t[]>(packet.size);
                memcpy(packet.data.get(), message.data(), packet.size);

                client.send_packet(packet, true, snow::_CHANNEL_RELIABLE);

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
        }
    });

    client_thread.join();
    server_thread.join();

    return 0;
}
