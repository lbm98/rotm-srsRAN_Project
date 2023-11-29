#include <vector>
#include <cstdint>

//
// TLV Serializer
//

using ByteIt = std::vector<uint8_t>::iterator;
using CByteIt = std::vector<uint8_t>::const_iterator;

struct PacketHeader {
    uint16_t packet_length;
    uint16_t packet_type;

    void serialize(ByteIt &ptr, ByteIt end) const;

    void deserialize(CByteIt &ptr, CByteIt end);

    std::size_t size() const;
};

struct PacketRecord {
    PacketHeader packet_header;
    std::vector<uint8_t> packet_data;

    void serialize(ByteIt &ptr, ByteIt end) const;

    void deserialize(CByteIt &ptr, CByteIt end);

    std::size_t size() const;
};

struct PacketList {
    std::vector<PacketRecord> packet_records;

    void serialize(ByteIt &ptr, ByteIt end) const;

    void deserialize(CByteIt &ptr, CByteIt end);

    //
    //  Helper methods
    //

    void serialize(std::vector<uint8_t> &buf) const;

    void deserialize(const std::vector<uint8_t> &buf);
};
