#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "srsran/asn1/rrc_nr/msg_common.h"


void pdu_to_hex_string(std::ostream& out, const std::vector<uint8_t>& pdu)
{
  // Set formatting
  out << std::hex << std::setfill('0');

  for (uint8_t byte : pdu) {
    out << std::setw(2) << static_cast<int>(byte);
  }

  // Reset formatting
  out << std::dec << std::setfill(' ');
}

void decode_ul_ccch_pdu(std::ostream& out, const std::vector<uint8_t>& pdu)
{
  pdu_to_hex_string(out, pdu);
  out << "\n\n";

  asn1::rrc_nr::ul_ccch_msg_s msg;
  asn1::cbit_ref              bref(srsran::byte_buffer{pdu});

  if (msg.unpack(bref) != asn1::SRSASN_SUCCESS) {
    out << "Failed to decode UL-CCCH pdu\n";
    return;
  }

  asn1::json_writer json_writer;
  msg.to_json(json_writer);
  out << json_writer.to_string();
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <in-filename> <out-filename>\n";
    return 1;
  }
  std::string in_filename  = argv[1];
  std::string out_filename = argv[2];

  std::ifstream in(in_filename, std::ios::binary);
  std::ofstream out(out_filename);

  if (not in.is_open()) {
    std::cerr << "Failed to open file for reading\n";
    return 1;
  }

  if (not out.is_open()) {
    std::cerr << "Failed to open file for writing\n";
    return 1;
  }

  std::vector<uint8_t> ul_ccch_pdu(std::istreambuf_iterator<char>(in), {});

  decode_ul_ccch_pdu(out, ul_ccch_pdu);
  out.close();

  return 0;
}