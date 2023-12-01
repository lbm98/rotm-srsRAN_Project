#include "tests/unittests/rrc/rrc_ue_test_helpers.h"

using namespace srsran;
using namespace srs_cu_cp;

class rrc_ue_fuzz : public rrc_ue_test_helper
{
public:
  rrc_ue_fuzz() { init(); }

  void run(byte_buffer rrc_setup_complete)
  {
    connect_amf();

    byte_buffer rrc_setup_request {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};

    receive_setup_request(rrc_setup_request);
    receive_setup_complete(rrc_setup_complete);

    // If the line below is missing, we crash on some inputs.
    // See tests/unittests/rrc/rrc_ue_setup_proc_test.cpp
    tick_timer();
  }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  // libfuzzer seems to test the 0 input by default
//  if (size == 0)
//    return 0;

  rrc_ue_fuzz ue;
  ue.run(byte_buffer(data, data + size));

  return 0;
}
