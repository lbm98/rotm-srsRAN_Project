#include <fstream>
#include <iostream>
#include <vector>

#include "srsran/asn1/rrc_nr/msg_common.h"

std::vector<uint8_t> read_bin(const std::string& filename)
{
  std::vector<uint8_t> bytes;
  std::ifstream in_file(filename, std::ios::binary);

  if (in_file.is_open()) {
    bytes = std::vector<uint8_t>(std::istreambuf_iterator<char>(in_file), {});
    in_file.close();
  } else {
    std::cerr << "Failed to open file for reading\n";
    exit(1);
  }

  return bytes;
}

void write_text(const std::string& filename, const std::string& text) {
  std::ofstream out_file(filename);

  if (out_file.is_open()) {
    out_file << text;
    out_file.close();
  } else {
    std::cerr << "Failed to open file for writing\n";
    exit(1);
  }
}

// See rrc_ue_impl::handle_ul_ccch_pdu
std::string decode_ul_ccch_pdu(std::vector<uint8_t> ul_ccch_pdu) {
  asn1::rrc_nr::ul_ccch_msg_s ul_ccch_msg;
  asn1::cbit_ref bref(srsran::byte_buffer{ul_ccch_pdu});

  if (ul_ccch_msg.unpack(bref) != asn1::SRSASN_SUCCESS) {
    return "Failed to decode UL-CCCH pdu\n";
  }

  asn1::json_writer json_writer;
  ul_ccch_msg.to_json(json_writer);
  std::string text = json_writer.to_string();
  return text;
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <in-filename> <out-filename>\n";
    return 1;
  }
  std::string in_filename = argv[1];
  std::string out_filename = argv[2];

  std::vector<uint8_t> ul_ccch_pdu = read_bin(in_filename);
  std::string text = decode_ul_ccch_pdu(ul_ccch_pdu);
  write_text(out_filename, text);

  return 0;
}