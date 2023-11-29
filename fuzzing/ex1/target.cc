#include "tests/unittests/rrc/rrc_ue_test_helpers.h"

#include "packet_list.h"

using namespace srsran;
using namespace srs_cu_cp;

class rrc_ue_fuzz : public rrc_ue_test_helper
{
public:
  rrc_ue_fuzz() { init(); }

  void run1(byte_buffer rrc_setup, byte_buffer rrc_setup_complete)
  {
    connect_amf();
    receive_setup_request(rrc_setup);
    receive_setup_complete(rrc_setup_complete);

    // If the line below is missing, we get a SEGFAULT on invalid inputs
    // Note that invalid inputs are not tested in tests/unittests/rrc/rrc_ue_setup_proc_test.cpp
    tick_timer();
  }

  void run2(byte_buffer rrc_setup)
  {
    connect_amf();
    receive_setup_request(rrc_setup);

    // If the line below is missing, we get a SEGFAULT on invalid inputs
    // Note that invalid inputs are not tested in tests/unittests/rrc/rrc_ue_setup_proc_test.cpp
    tick_timer();
  }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  // libfuzzer seems to test the 0 input by default
  if (size == 0)
    return 0;

  PacketList packet_list{};
  try {
    std::vector<uint8_t> buf{data, data + size};
    packet_list.deserialize(buf);
  } catch (std::runtime_error& err) {
    // If the input is valid,
    // then all variations created by mutations must also be valid.
    assert(false);
  }

  auto& packet_records = packet_list.packet_records;

  // If the input encodes 2 packets,
  // then all variations created by mutations must also encode 2 packets.
  assert(packet_records.size() == 2);

  auto& rrc_setup          = packet_records[0].packet_data;
  auto& rrc_setup_complete = packet_records[1].packet_data;

  rrc_ue_fuzz ue;
  ue.run1(byte_buffer(rrc_setup), byte_buffer(rrc_setup_complete));

  return 0;
}

//int main()
//{
//  std::vector<uint8_t> packet_data1   = {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};
//  uint16_t             packet_type1   = 1;
//  uint16_t             packet_length1 = packet_data1.size();
//  PacketHeader         packet_hdr1{packet_length1, packet_type1};
//  PacketRecord         record1{packet_hdr1, packet_data1};
//
//  std::vector<uint8_t> packet_data2 = {
//      0x00, 0x00, 0x10, 0xc0, 0x10, 0x00, 0x08, 0x27, 0x27, 0xe0, 0x1c, 0x3f, 0xf1, 0x00, 0xc0, 0x47, 0xe0, 0x04, 0x13,
//      0x90, 0x00, 0xbf, 0x20, 0x2f, 0x89, 0x98, 0x00, 0x04, 0x10, 0x00, 0x00, 0x00, 0xf2, 0xe0, 0x4f, 0x07, 0x0f, 0x07,
//      0x07, 0x10, 0x05, 0x17, 0xe0, 0x04, 0x13, 0x90, 0x00, 0xbf, 0x20, 0x2f, 0x89, 0x98, 0x00, 0x04, 0x10, 0x00, 0x00,
//      0x00, 0xf1, 0x00, 0x10, 0x32, 0xe0, 0x4f, 0x07, 0x0f, 0x07, 0x02, 0xf1, 0xb0, 0x80, 0x10, 0x02, 0x7d, 0xb0, 0x00,
//      0x00, 0x00, 0x00, 0x80, 0x10, 0x1b, 0x66, 0x90, 0x00, 0x00, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00,
//      0x00, 0x05, 0x20, 0x2f, 0x89, 0x90, 0x00, 0x00, 0x11, 0x70, 0x7f, 0x07, 0x0c, 0x04, 0x01, 0x98, 0x0b, 0x01, 0x80,
//      0x10, 0x17, 0x40, 0x00, 0x09, 0x05, 0x30, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00};
//  uint16_t     packet_type2   = 2;
//  uint16_t     packet_length2 = packet_data2.size();
//  PacketHeader packet_hdr2{packet_length2, packet_type2};
//  PacketRecord         record2{packet_hdr2, packet_data2};
//
//  PacketList packet_list{{record1, record2}};
//
//  std::vector<uint8_t> buf{};
//  packet_list.serialize(buf);
//
//  LLVMFuzzerTestOneInput(buf.data(), buf.size());
//
//  return 0;
//}
