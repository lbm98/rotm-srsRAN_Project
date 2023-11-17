#include "tests/unittests/rrc/rrc_ue_test_helpers.h"
#include <fstream>

using namespace srsran;
using namespace srs_cu_cp;

class rrc_ue_fuzz : public rrc_ue_test_helper
{
public:
  rrc_ue_fuzz() { init(); }

  void run2(byte_buffer pdu)
  {
    connect_amf();
    receive_setup_request(pdu);
    tick_timer();
  }
};

// https://github.com/AFLplusplus/AFLplusplus/blob/stable/utils/persistent_mode/persistent_demo_new.c

__AFL_FUZZ_INIT();

// To ensure checks are not optimized out it is recommended to disable
// code optimization for the fuzzer harness main()
#pragma clang optimize off

int main(int argc, char* argv[])
{
#ifdef __AFL_HAVE_MANUAL_CONTROL
  __AFL_INIT();
#endif

  unsigned char *buf = __AFL_FUZZ_TESTCASE_BUF;

  while (__AFL_LOOP(10000)) {
    int len = __AFL_FUZZ_TESTCASE_LEN;

    byte_buffer pdu(buf, buf + len);

    rrc_ue_fuzz ue;
    ue.run2(std::move(pdu));
  }

  return 0;
}