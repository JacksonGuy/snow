#include <memory>
#include <stdlib.h>
#include <cstring>
#include <iostream>

#include "enet/enet.h"

#include "net/packet.h"
#include "core/utils.h"

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

        this->data = std::make_unique<uint8_t[]>(this->size);
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

            this->data = std::make_unique<uint8_t[]>(this->size);
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
        uint8_t* bytes = (uint8_t*)event->packet->data;
        size_t size = event->packet->dataLength;

        if (size <= 0) return;
        this->deserialize(bytes);
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
    uint64_t Packet::get_size() const noexcept {
        uint64_t size_total = 0;

        size_total += _UUID_SIZE;
        size_total += sizeof(this->size);

        size_total += this->size;

        return size_total;
    }

    /*
     * Serializes the packet contents into a byte array.
     */
    uint8_t* Packet::serialize() const {
        uint64_t size_total = this->get_size();
        uint8_t* buffer = (uint8_t*)malloc(size_total);
        uint64_t offset = 0;

        memcpy((char*)buffer, this->uuid, _UUID_SIZE);
        offset += _UUID_SIZE;

        serialize_uint64_t(buffer + offset, this->size);
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
    bool Packet::deserialize(const uint8_t* buffer) {
        uint64_t offset = 0;

        // UUID
        memcpy(this->uuid, buffer, _UUID_SIZE);
        offset += _UUID_SIZE;

        // Size
        this->size = deserialize_uint64_t(buffer + offset);
        offset += sizeof(this->size);

        // Free any data currently being stored
        if (this->data != nullptr) {
            this->data.reset();
        }

        // Now we copy the data
        this->data = std::make_unique<uint8_t[]>(this->size);
        memcpy(this->data.get(), (buffer + offset), this->size);

        return true;
    }
}
