#include <stdexcept>
#include <cstring>
#include <sstream>

#include "packet_list.h"

void PacketHeader::serialize(ByteIt &ptr, ByteIt end) const {
    int size = sizeof(*this);
    if (end - ptr < size)
        throw std::runtime_error("PacketHeader::serialize: PacketHeader does not fit in provided buffer");

    std::memcpy(&(*ptr), this, sizeof(*this));
    ptr += sizeof(*this);
}

void PacketHeader::deserialize(CByteIt &ptr, CByteIt end) {
    int size = sizeof(*this);
    if (end - ptr < size)
        throw std::runtime_error("PacketHeader::deserialize: Not enough data provided to construct PacketHeader");

    std::memcpy(this, &(*ptr), sizeof(*this));
    ptr += sizeof(*this);
}

std::size_t PacketHeader::size() const {
    return sizeof(*this);
}

void PacketRecord::serialize(ByteIt &ptr, ByteIt end) const {
    packet_header.serialize(ptr, end);

    if (end - ptr < packet_header.packet_length) {
        std::stringstream ss;
        ss << "PacketRecord::serialize: "
           << "PacketRecord.packet_data (" << packet_header.packet_length << ") "
           << "does not fit in provided buffer (" << end - ptr << ")";
        throw std::runtime_error(ss.str());
    }

    std::copy(packet_data.begin(), packet_data.begin() + packet_header.packet_length, ptr);
    ptr += packet_header.packet_length;
}

void PacketRecord::deserialize(CByteIt &ptr, CByteIt end) {
    packet_header.deserialize(ptr, end);

    if (end - ptr < packet_header.packet_length) {
        std::stringstream ss;
        ss << "PacketRecord::deserialize: "
           << "Not enough data provided (" << end - ptr << ") "
           << "to construct PacketRecord.packet_data (" << packet_header.packet_length << ") ";
        throw std::runtime_error(ss.str());
    }

    packet_data.resize(packet_header.packet_length);
    std::copy(ptr, ptr + packet_header.packet_length, packet_data.begin());
    ptr += packet_header.packet_length;
}

std::size_t PacketRecord::size() const {
    return packet_header.size() + packet_header.packet_length;
}


void PacketList::serialize(ByteIt &ptr, ByteIt end) const {
    for (const auto &packet: packet_records) {
        packet.serialize(ptr, end);
    }
}

void PacketList::deserialize(CByteIt &ptr, CByteIt end) {
    while (ptr != end) {
        PacketRecord packet{};
        packet.deserialize(ptr, end);
        packet_records.push_back(packet);
    }
}

void PacketList::serialize(std::vector<uint8_t> &buf) const {
    size_t size = 0;
    for (const auto &packet_record: packet_records)
        size += packet_record.size();
    buf.resize(size);

    auto ptr = buf.begin();
    serialize(ptr, buf.end());
}

void PacketList::deserialize(const std::vector<uint8_t> &buf) {
    auto ptr = buf.cbegin();
    deserialize(ptr, buf.cend());
}


