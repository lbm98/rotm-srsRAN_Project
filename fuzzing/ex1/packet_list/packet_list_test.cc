#include <cassert>
#include <stdexcept>

#include "packet_list.h"

void assert_eq_packet_header(const PacketHeader &hdr, const PacketHeader &hdr2) {
    assert(hdr.packet_length == hdr2.packet_length);
    assert(hdr.packet_type == hdr2.packet_type);
}

void assert_eq_packet_record(const PacketRecord &record, const PacketRecord &record2) {
    assert_eq_packet_header(record.packet_header, record2.packet_header);

    assert(record.packet_data.size() == record2.packet_data.size());
    for (std::size_t i = 0; i < record.packet_data.size(); i++)
        assert(record.packet_data[i] == record2.packet_data[i]);
}

void assert_eq_packet_list(const PacketList &pl, const PacketList &pl2) {
    assert(pl.packet_records.size() == pl2.packet_records.size());

    for (std::size_t i = 0; i < pl.packet_records.size(); i++)
        assert_eq_packet_record(pl.packet_records[i], pl2.packet_records[i]);
}

void test_packet_header() {
    PacketHeader hdr{4, 2};

    std::vector<uint8_t> buf(hdr.size());
    auto ptr = buf.begin();
    hdr.serialize(ptr, buf.end());

    auto cptr = buf.cbegin();
    PacketHeader hdr2{};
    hdr2.deserialize(cptr, buf.cend());

    assert_eq_packet_header(hdr, hdr2);
}

void test_packet_record() {
    PacketHeader hdr{4, 2};
    std::vector<uint8_t> data{0, 1, 2, 3};
    PacketRecord record{hdr, data};

    std::vector<uint8_t> buf(record.size());
    auto ptr = buf.begin();
    record.serialize(ptr, buf.end());

    auto cptr = buf.cbegin();
    PacketRecord record2{};
    record2.deserialize(cptr, buf.cend());

    assert_eq_packet_record(record, record2);
}

void test_happy_day() {
    PacketHeader hdr{4, 2};
    std::vector<uint8_t> data{0, 1, 2, 3};
    PacketRecord record{hdr, data};
    PacketHeader hdr2{3, 1};
    std::vector<uint8_t> data2{6, 7, 8};
    PacketRecord record2{hdr2, data2};
    PacketList pl{{record, record2}};

    std::vector<uint8_t> buf{};
    pl.serialize(buf);

    PacketList pl2{};
    pl2.deserialize(buf);

    assert_eq_packet_list(pl, pl2);
}

void test_empty() {
    PacketList pl{};

    std::vector<uint8_t> buf{};
    pl.serialize(buf);

    PacketList pl2{};
    pl2.deserialize(buf);

    assert_eq_packet_list(pl, pl2);
}

void test_invalid() {
    PacketHeader hdr{5, 2};
    std::vector<uint8_t> data{0, 1, 2, 3};
    PacketRecord record{hdr, data};
    PacketList pl{{record}};

    try {
        std::vector<uint8_t> buf{};
        pl.serialize(buf);
    } catch (std::runtime_error& e) {

    }
}

int main() {
    test_packet_header();
    test_packet_record();
    test_happy_day();
    test_empty();
    test_invalid();
}