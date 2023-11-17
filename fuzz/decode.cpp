#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "srsran/asn1/rrc_nr/msg_common.h"

std::vector<uint8_t> read_bin(const std::string& filename)
{
  std::vector<uint8_t> bytes;
  std::ifstream        in_file(filename, std::ios::binary);

  if (in_file.is_open()) {
    bytes = std::vector<uint8_t>(std::istreambuf_iterator<char>(in_file), {});
    in_file.close();
  } else {
    std::cerr << "Failed to open file for reading\n";
    exit(1);
  }

  return bytes;
}

void write_text(const std::string& filename, const std::string& text)
{
  std::ofstream out_file(filename);

  if (out_file.is_open()) {
    out_file << text;
    out_file.close();
  } else {
    std::cerr << "Failed to open file for writing\n";
    exit(1);
  }
}

std::string pdu_to_hex_string(const std::vector<uint8_t>& pdu)
{
  std::ostringstream oss;
  oss << std::hex << std::setfill('0');

  for (uint8_t byte : pdu) {
    oss << std::setw(2) << static_cast<int>(byte);
  }
  return oss.str();
}

//
// See rrc_ue_impl::handle_ul_ccch_pdu
//
std::string pdu_to_json_string(const std::vector<uint8_t>& pdu)
{
  asn1::rrc_nr::ul_ccch_msg_s ul_ccch_msg;
  asn1::cbit_ref              bref(srsran::byte_buffer{pdu});

  if (ul_ccch_msg.unpack(bref) != asn1::SRSASN_SUCCESS) {
    return "Failed to decode UL-CCCH pdu\n";
  }

  asn1::json_writer json_writer;
  ul_ccch_msg.to_json(json_writer);
  return json_writer.to_string();
}

std::string decode_ul_ccch_pdu(const std::vector<uint8_t>& ul_ccch_pdu)
{
  std::string hex_string  = pdu_to_hex_string(ul_ccch_pdu);
  std::string json_string = pdu_to_json_string(ul_ccch_pdu);

  std::ostringstream oss;
  oss << hex_string << '\n' << json_string;
  return oss.str();
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <in-filename> <out-filename>\n";
    return 1;
  }
  std::string in_filename  = argv[1];
  std::string out_filename = argv[2];

  std::vector<uint8_t> ul_ccch_pdu = read_bin(in_filename);
  std::string          text        = decode_ul_ccch_pdu(ul_ccch_pdu);
  write_text(out_filename, text);

  return 0;
}