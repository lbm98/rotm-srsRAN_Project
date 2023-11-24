#include "pcap_de.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

// Maybe use
// static int pcap_next_packet(pcap_t *p, struct pcap_pkthdr *hdr, u_char **data)
// from https://github.com/the-tcpdump-group/libpcap

struct pcaprec_hdr_t {
  unsigned int ts_sec;   /// timestamp seconds
  unsigned int ts_usec;  /// timestamp microseconds
  unsigned int incl_len; /// number of octets of packet saved in file
  unsigned int orig_len; /// actual length of packet
};

void encode_pcap(const std::vector<std::vector<uint8_t>>& packets, uint8_t** data, std::size_t* size)
{
  for (const auto& packet : packets) {
    *size += sizeof(struct pcaprec_hdr_t);
    *size += packet.size();
  }
  *data        = static_cast<uint8_t*>(calloc(*size, 1));
  uint8_t* ptr = *data;

  for (const auto& packet : packets) {
    pcaprec_hdr_t hdr;
    hdr.ts_sec   = 0;
    hdr.ts_usec  = 0;
    hdr.incl_len = packet.size();
    hdr.orig_len = 0;

    std::memcpy(ptr, &hdr, sizeof(struct pcaprec_hdr_t));
    ptr += sizeof(struct pcaprec_hdr_t);

    std::memcpy(ptr, packet.data(), packet.size());
    ptr += packet.size();
  }

  uint8_t* end = *data + *size;
  assert(ptr == end);
}

std::vector<std::vector<uint8_t>> decode_pcap(const uint8_t* data, std::size_t size)
{
  std::vector<std::vector<uint8_t>> result;

  const uint8_t* ptr = data;
  const uint8_t* end = data + size;

  while (ptr < end) {
    const auto*  hdr      = reinterpret_cast<const pcaprec_hdr_t*>(ptr);
    unsigned int hdr_size = hdr->incl_len;
    ptr += sizeof(struct pcaprec_hdr_t);
    std::vector<uint8_t> buf(ptr, ptr + hdr_size);
    result.push_back(buf);
    ptr += hdr_size;
  }
  assert(ptr == end);

  return result;
}
