#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include "packet_list.h"

#include "srsran/asn1/rrc_nr/msg_common.h"
#include "srsran/asn1/rrc_nr/ul_dcch_msg.h"


void pdu_to_hex_string(std::ostream& oss, const std::vector<uint8_t>& pdu)
{
  // Set formatting
  oss << std::hex << std::setfill('0');

  for (uint8_t byte : pdu) {
    oss << std::setw(2) << static_cast<int>(byte);
  }

  // Reset formatting
  oss << std::dec << std::setfill(' ');
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

void decode_ul_dcch_pdu(std::ostream& out, const std::vector<uint8_t>& pdu)
{
  pdu_to_hex_string(out, pdu);
  out << "\n\n";

  asn1::rrc_nr::ul_dcch_msg_s msg;
  asn1::cbit_ref              bref(srsran::byte_buffer{pdu});

  if (msg.unpack(bref) != asn1::SRSASN_SUCCESS or msg.msg.type().value != asn1::rrc_nr::ul_dcch_msg_type_c::types_opts::c1) {
    out << "Failed to decode UL-DCCH pdu\n";
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

  auto buf = std::vector<uint8_t>(std::istreambuf_iterator<char>(in), {});

  PacketList packet_list{};
  packet_list.deserialize(buf);

  auto& packet_records = packet_list.packet_records;
  assert(packet_records.size() == 2);
  auto& ul_ccch_pdu = packet_records[0].packet_data;
  auto& ul_dcch_pdu = packet_records[1].packet_data;

  decode_ul_ccch_pdu(out, ul_ccch_pdu);
  out << "\n\n";
  decode_ul_dcch_pdu(out, ul_dcch_pdu);

  return 0;
}