#include "tests/unittests/rrc/rrc_ue_test_helpers.h"

#include "pcap_de.h"

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
  }

  void run2(byte_buffer rrc_setup)
  {
    connect_amf();
    receive_setup_request(rrc_setup);
    tick_timer();
  }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  std::vector<std::vector<uint8_t>> packets = decode_pcap(data, size);

  std::vector<uint8_t> rrc_setup          = packets[0];
  std::vector<uint8_t> rrc_setup_complete = packets[1];

  rrc_ue_fuzz ue;
  ue.run1(byte_buffer(rrc_setup), byte_buffer(rrc_setup_complete));

  return 0;
}

#ifdef TEST_TARGET
int main()
{
  uint8_t buf1[] = {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};

  uint8_t buf2[] = {0x00, 0x00, 0x10, 0xc0, 0x10, 0x00, 0x08, 0x27, 0x27, 0xe0, 0x1c, 0x3f, 0xf1, 0x00, 0xc0, 0x47,
                    0xe0, 0x04, 0x13, 0x90, 0x00, 0xbf, 0x20, 0x2f, 0x89, 0x98, 0x00, 0x04, 0x10, 0x00, 0x00, 0x00,
                    0xf2, 0xe0, 0x4f, 0x07, 0x0f, 0x07, 0x07, 0x10, 0x05, 0x17, 0xe0, 0x04, 0x13, 0x90, 0x00, 0xbf,
                    0x20, 0x2f, 0x89, 0x98, 0x00, 0x04, 0x10, 0x00, 0x00, 0x00, 0xf1, 0x00, 0x10, 0x32, 0xe0, 0x4f,
                    0x07, 0x0f, 0x07, 0x02, 0xf1, 0xb0, 0x80, 0x10, 0x02, 0x7d, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x80,
                    0x10, 0x1b, 0x66, 0x90, 0x00, 0x00, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
                    0x05, 0x20, 0x2f, 0x89, 0x90, 0x00, 0x00, 0x11, 0x70, 0x7f, 0x07, 0x0c, 0x04, 0x01, 0x98, 0x0b,
                    0x01, 0x80, 0x10, 0x17, 0x40, 0x00, 0x09, 0x05, 0x30, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00};

  std::vector<uint8_t> packet1(std::begin(buf1), std::end(buf1));
  std::vector<uint8_t> packet2(std::begin(buf2), std::end(buf2));

  std::vector<std::vector<uint8_t>> packets;
  packets.push_back(packet1);
  packets.push_back(packet2);

  uint8_t* data = nullptr;
  size_t   size = 0;
  encode_pcap(packets, &data, &size);

  LLVMFuzzerTestOneInput(data, size);

  return 0;
}
#endif