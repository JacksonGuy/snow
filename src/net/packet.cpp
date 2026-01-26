#include <memory>
#include <stdlib.h>
#include <cstring>
#include <iostream>

#include "net/packet.hpp"
#include "core/utils.hpp"
#include "enet/enet.h"

namespace snow {
    /*
     * Default constructor.
     * Data buffer is nullptr.
     */
    Packet::Packet() {
        strcpy(this->uuid, Packet::default_uuid);
        this->size = 0;
        this->data = nullptr;
    }

    /*
     * Copy constructor.
     * Copies the underlying data buffer.
     */
    Packet::Packet(const Packet& packet) {
        strcpy(this->uuid, packet.uuid);
        this->size = packet.size;

        this->data = std::make_unique<u8[]>(this->size);
        memcpy(this->data.get(), packet.data.get(), this->size);
    }

    /*
     * Move constructor.
     * Takes ownership of the underlying data buffer.
     */
    Packet::Packet(Packet&& packet) noexcept {
        strcpy(this->uuid, packet.uuid);
        this->size = packet.size;
        this->data = std::move(packet.data);
    }

    /*
     * Copy assignment.
     * Copies the underlying data buffer
     */
    Packet& Packet::operator=(const Packet& packet) {
        if (this != &packet) {
            strcpy(this->uuid, packet.uuid);
            this->size = packet.size;

            this->data = std::make_unique<u8[]>(this->size);
            memcpy(this->data.get(), packet.data.get(), this->size);
        }
        return *this;
    }

    /*
     * Move assignment operator.
     * Takes ownership of the underlying data buffer.
     */
    Packet& Packet::operator=(Packet&& packet) noexcept {
        if (this != &packet) {
            strcpy(this->uuid, packet.uuid);
            this->size = packet.size;

            this->data = std::move(packet.data);
        }
        return *this;
    }

    /*
     * Construct packet from ENet event.
     */
    Packet::Packet(ENetEvent* event) {
        u8* bytes = (u8*)event->packet->data;
        size_t size = event->packet->dataLength;

        if (size <= 0) return;
        this->Deserialize(bytes);
    }

    /*
     * Destructor.
     */
    Packet::~Packet() {}

    /*
     * Return the total size of the packet in bytes.
     * This does NOT return the value of the internal size variable.
     * This should only be used when serializing the packet.
     */
    u64 Packet::get_size() const noexcept {
        u64 size_total = 0;

        size_total += _UUID_SIZE;
        size_total += sizeof(this->size);

        size_total += this->size;

        return size_total;
    }

    /*
     * Serializes the packet contents into a byte array.
     */
    u8* Packet::Serialize() const {
        u64 size_total = this->get_size();
        u8* buffer = (u8*)malloc(size_total);
        u64 offset = 0;

        memcpy((char*)buffer, this->uuid, _UUID_SIZE);
        offset += _UUID_SIZE;

        serialize_u64(buffer + offset, this->size);
        offset += sizeof(this->size);

        if (this->data != nullptr && this->size > 0) {
            memcpy((buffer + offset), this->data.get(), this->size);
        }

        return buffer;
    }

    /*
     * Deserialize the given byte array into the packet object.
     * Modifies the current packet object.
     */
    bool Packet::Deserialize(const u8* buffer) {
        u64 offset = 0;

        // UUID
        memcpy(this->uuid, buffer, _UUID_SIZE);
        offset += _UUID_SIZE;

        // Size
        this->size = deserialize_u64(buffer + offset);
        offset += sizeof(this->size);

        // Free any data currently being stored
        if (this->data != nullptr) {
            this->data.reset();
        }

        // Now we copy the data
        this->data = std::make_unique<u8[]>(this->size);
        memcpy(this->data.get(), (buffer + offset), this->size);

        return true;
    }
}
