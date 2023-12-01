#include <fstream>
#include <iostream>
#include <vector>

int main()
{
  std::ofstream out("inputs/input.bin", std::ios::binary);

  if (not out.is_open()) {
    std::cerr << "Unable to open the file!\n";
    return 1;
  }

  std::vector<uint8_t> rrc_setup_request_pdu = {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};

  out.write(reinterpret_cast<const char*>(rrc_setup_request_pdu.data()), rrc_setup_request_pdu.size());
  out.close();

  return 0;
}