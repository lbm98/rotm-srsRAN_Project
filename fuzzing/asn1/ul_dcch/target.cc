#include "srsran/asn1/rrc_nr/ul_dcch_msg.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  asn1::rrc_nr::ul_dcch_msg_s msg;
  asn1::cbit_ref              bref(srsran::byte_buffer{data, data + size});
  msg.unpack(bref);
  return 0;
}