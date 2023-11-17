#include "tests/unittests/rrc/rrc_ue_test_helpers.h"
#include <fstream>

using namespace srsran;
using namespace srs_cu_cp;

class rrc_ue_fuzz : public rrc_ue_test_helper
{
public:
  rrc_ue_fuzz() { init(); }

  void run1(std::vector<uint8_t> rrc_setup, std::vector<uint8_t> rrc_setup_complete)
  {
    connect_amf();
    receive_setup_request(rrc_setup);
    receive_setup_complete(rrc_setup_complete);
  }

  void run2(byte_buffer pdu)
  {
    connect_amf();
    receive_setup_request(pdu);
    tick_timer();
  }
};

std::vector<uint8_t> read_bin(const std::string& filename)
{
  std::vector<uint8_t> bytes;

  std::ifstream in_file(filename, std::ios::binary);

  if (in_file.is_open()) {
    bytes = std::vector<uint8_t>(std::istreambuf_iterator<char>(in_file), {});
    in_file.close();
  } else {
    std::cerr << "Unable to open file for reading!";
  }

  return bytes;
}

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <filename>\n";
    return 1;
  }

//  std::vector<uint8_t> rrc_setup = read_bin(argv[1]);

  unsigned char arr[] = {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};
  unsigned char *buf = arr;
  int len = 6;

  byte_buffer pdu(buf, buf + len);

  rrc_ue_fuzz ue;
  ue.run2(std::move(pdu));

  return 0;
}