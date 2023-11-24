#include <vector>

#include <cstdint>

void encode_pcap(const std::vector<std::vector<uint8_t>>& packets, uint8_t** data, std::size_t* size);

std::vector<std::vector<uint8_t>> decode_pcap(const uint8_t* data, std::size_t size);
