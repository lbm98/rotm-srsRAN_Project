#include "pcap_de.h"

#include <cassert>
#include <cstdlib>

void assert_vec_eq(std::vector<uint8_t> v1, std::vector<uint8_t> v2)
{
  assert(v1.size() == v2.size());

  for (std::size_t i = 0; i < v1.size(); i++) {
    assert(v1[i] == v2[i]);
  }
}

int main()
{
  uint8_t buf1[] = {0x1d, 0xec, 0x89, 0xd0, 0x57, 0x66};

  uint8_t buf2[] = {0x00, 0x00, 0x10, 0xc0, 0x10, 0x00, 0x08, 0x27, 0x27, 0xe0, 0x1c, 0x3f, 0xf1, 0x00, 0xc0, 0x47,
                    0xe0, 0x04, 0x13, 0x90, 0x00, 0xbf, 0x20, 0x2f, 0x89, 0x98, 0x00, 0x04, 0x10, 0x00, 0x00, 0x00,
                    0xf2, 0xe0, 0x4f, 0x07, 0x0f, 0x07, 0x07, 0x10, 0x05, 0x17, 0xe0, 0x04, 0x13, 0x90, 0x00, 0xbf,
                    0x20, 0x2f, 0x89, 0x98, 0x00, 0x04, 0x10, 0x00, 0x00, 0x00, 0xf1, 0x00, 0x10, 0x32, 0xe0, 0x4f,
                    0x07, 0x0f, 0x07, 0x02, 0xf1, 0xb0, 0x80, 0x10, 0x02, 0x7d, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x80,
                    0x10, 0x1b, 0x66, 0x90, 0x00, 0x00, 0x00, 0x00, 0x80, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
                    0x05, 0x20, 0x2f, 0x89, 0x90, 0x00, 0x00, 0x11, 0x70, 0x7f, 0x07, 0x0c, 0x04, 0x01, 0x98, 0x0b,
                    0x01, 0x80, 0x10, 0x17, 0x40, 0x00, 0x09, 0x05, 0x30, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00};

  std::vector<uint8_t> packet1(std::begin(buf1), std::end(buf1));
  std::vector<uint8_t> packet2(std::begin(buf2), std::end(buf2));

  std::vector<std::vector<uint8_t>> packets;
  packets.push_back(packet1);
  packets.push_back(packet2);

  uint8_t* data = nullptr;
  std::size_t   size = 0;

  encode_pcap(packets, &data, &size);
  std::vector<std::vector<uint8_t>> packets_decode = decode_pcap(data, size);

  assert(packets_decode.size() == 2);
  std::vector<uint8_t> packet1_decode = packets_decode[0];
  std::vector<uint8_t> packet2_decode = packets_decode[1];

  assert_vec_eq(packet1, packet1_decode);
  assert_vec_eq(packet2, packet2_decode);

  free(data);
}