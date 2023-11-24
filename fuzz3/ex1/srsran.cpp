#include "tests/unittests/rrc/rrc_ue_test_helpers.h"

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

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  byte_buffer pdu(data, data + size);
  rrc_ue_fuzz ue;
  ue.run2(std::move(pdu));
  return 0;
}

//int main() {
//  uint8_t data[] = {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};
//  size_t size = sizeof(data);
//
//  byte_buffer pdu(data, data + size);
//  rrc_ue_fuzz ue;
//  ue.run2(std::move(pdu));
//  return 0;
//}