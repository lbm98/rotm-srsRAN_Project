#include "lib/pcap/dlt_pcap_impl.h"
#include "rrc_ue_test_helpers.h"

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
//    srslog::flush();
  }

  void run2(std::vector<uint8_t> rrc_setup)
  {
    connect_amf();
    receive_setup_request(rrc_setup);
    tick_timer();
//    srslog::flush();
  }
};

void write_bin(const std::string& filename, const std::vector<uint8_t>& bytes)
{
  std::ofstream out_file(filename, std::ios::binary);

  if (out_file.is_open()) {
    out_file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    out_file.close();
  } else {
    std::cerr << "Unable to open file for writing!\n";
  }
}

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

void read_write_bin_test()
{
  std::vector<uint8_t> rrc_setup = {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};
  write_bin("/tmp/rrc_setup.bin", rrc_setup);
  auto rrc_setup_2 = read_bin("/tmp/rrc_setup.bin");

  // hexdump /tmp/rrc_setup.bin

  assert(rrc_setup.size() == rrc_setup_2.size());
  for (std::size_t i = 0; i < rrc_setup.size(); i++) {
    assert(rrc_setup[i] == rrc_setup_2[i]);
  }
}

void generate_input_corpus()
{
  std::vector<uint8_t> rrc_setup = {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};

  std::vector<uint8_t> rrc_setup_complete = {
      0x00, 0x00, 0x10, 0xc0, 0x10, 0x00, 0x08, 0x27, 0x27, 0xe0, 0x1c, 0x3f, 0xf1, 0x00, 0xc0, 0x47, 0xe0, 0x04, 0x13,
      0x90, 0x00, 0xbf, 0x20, 0x2f, 0x89, 0x98, 0x00, 0x04, 0x10, 0x00, 0x00, 0x00, 0xf2, 0xe0, 0x4f, 0x07, 0x0f, 0x07,
      0x07, 0x10, 0x05, 0x17, 0xe0, 0x04, 0x13, 0x90, 0x00, 0xbf, 0x20, 0x2f, 0x89, 0x98, 0x00, 0x04, 0x10, 0x00, 0x00,
      0x00, 0xf1, 0x00, 0x10, 0x32, 0xe0, 0x4f, 0x07, 0x0f, 0x07, 0x02, 0xf1, 0xb0, 0x80, 0x10, 0x02, 0x7d, 0xb0, 0x00,
      0x00, 0x00, 0x00, 0x80, 0x10, 0x1b, 0x66, 0x90, 0x00, 0x00, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00,
      0x00, 0x05, 0x20, 0x2f, 0x89, 0x90, 0x00, 0x00, 0x11, 0x70, 0x7f, 0x07, 0x0c, 0x04, 0x01, 0x98, 0x0b, 0x01, 0x80,
      0x10, 0x17, 0x40, 0x00, 0x09, 0x05, 0x30, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00};

  write_bin("rrc_setup.bin", rrc_setup);
  write_bin("rrc_setup_complete.bin", rrc_setup_complete);
}

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <filename>\n";
    return 1;
  }

  std::vector<uint8_t> bytes = read_bin(argv[1]);

//  srslog::init();
  rrc_ue_fuzz ue;

  ue.run2(bytes);

  //  ue.run1(rrc_setup, rrc_setup_complete);
  //  ue.run2(rrc_setup);

  //  read_write_bin_test();
  //  generate_input_corpus();

  //  dlt_pcap_impl pcap;
  //  mac_pcap_writer.close();

  return 0;
}