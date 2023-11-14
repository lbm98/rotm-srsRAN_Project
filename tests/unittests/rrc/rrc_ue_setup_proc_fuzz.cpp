#include "rrc_ue_test_helpers.h"

using namespace srsran;
using namespace srs_cu_cp;

class rrc_ue_fuzz : public rrc_ue_test_helper {
public:
  void test(std::array<uint8_t, 6> buffer) {
    init();
    connect_amf();
    receive_setup_request(buffer);
    srslog::flush();
  }
};

int main() {
  srslog::init();

  rrc_ue_fuzz ue;

  std::array<uint8_t, 6> buffer = {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};

  ue.test(buffer);

  std::cout << "hello";
}