#include <memory>
#include <stdlib.h>
#include <cstring>

#include "net/packet.hpp"
#include "core/utils.hpp"

namespace snow {
    /*
     * Default constructor.
     * Data buffer is nullptr.
     */
    Packet::Packet() {
        this->type = 0;
        this->timestamp = 0;
        this->priority = 0;
        this->size = 0;
        this->data = nullptr;
    }

    /*
     * Copy constructor.
     * Copies the underlying data buffer.
     */
    Packet::Packet(const Packet& packet) {
        this->type = packet.type;
        this->timestamp = packet.timestamp;
        this->priority = packet.priority;
        this->size = packet.size;

        this->data = std::make_unique<u8>(this->size);
        memcpy(this->data.get(), packet.data.get(), this->size);
    }

    /*
     * Move constructor.
     * Takes ownership of the underlying data buffer.
     */
    Packet::Packet(Packet&& packet) noexcept {
        this->type = packet.type;
        this->timestamp = packet.timestamp;
        this->priority = packet.priority;
        this->size = packet.size;
        this->data = std::move(packet.data);
    }

    /*
     * Copy assignment.
     * Copies the underlying data buffer
     */
    Packet& Packet::operator=(const Packet& packet) {
        if (this != &packet) {
            this->type = packet.type;
            this->timestamp = packet.timestamp;
            this->priority = packet.priority;
            this->size = packet.size;

            this->data = std::make_unique<u8>(this->size);
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
            this->type = packet.type;
            this->timestamp = packet.timestamp;
            this->priority = packet.priority;
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
     */
    u64 Packet::get_size() const noexcept {
        u64 size_total = 0;

        size_total += sizeof(this->type);
        size_total += sizeof(this->timestamp);
        size_total += sizeof(this->priority);
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

        // Write data to buffer and move over by 8 bytes (64 bits)
        serialize_u64(buffer, this->type);      buffer += 8;
        serialize_u64(buffer, this->timestamp); buffer += 8;
        serialize_u64(buffer, this->priority);  buffer += 8;
        serialize_u64(buffer, this->size);      buffer += 8;

        if (this->data != nullptr && this->size > 0) {
            memcpy(buffer, this->data.get(), this->size);
        }

        return buffer;
    }

    /*
     * Deserialize the given byte array into the packet object.
     * Modifies the current packet object.
     */
    bool Packet::Deserialize(const u8* buffer) {
        u64 offset = 0;

        this->type = deserialize_u64(buffer + offset);        offset += 8;
        this->timestamp = deserialize_u64(buffer + offset);   offset += 8;
        this->priority = deserialize_u64(buffer + offset);    offset += 8;
        this->size = deserialize_u64(buffer + offset);        offset += 8;

        // We free the current data stored locally
        if (this->data != nullptr) {
            this->data.reset();
        }

        // Now we copy the data
        this->data = std::make_unique<u8>(this->size);
        memcpy(this->data.get(), (buffer + offset), this->size);

        return true;
    }
}
