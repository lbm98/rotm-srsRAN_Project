#include <random>
#include <cassert>
#include <iostream>

#include "packet_list.h"


// See https://github.com/llvm/llvm-project/blob/main/compiler-rt/lib/fuzzer/FuzzerInterface.h
extern "C" size_t LLVMFuzzerMutate(uint8_t *data, size_t size, size_t max_size);


// Optional user-provided custom mutator.
// Mutates raw data in [Data, Data+Size) inplace.
// Returns the new size, which is not greater than MaxSize.
// Given the same Seed produces the same mutation.

extern "C" size_t LLVMFuzzerCustomMutator(uint8_t *data, size_t size, size_t max_size, unsigned int seed) {

    if (size == 0)
        return 0;

    // Deserialize from buffer
    std::vector<uint8_t> buf{data, data + size};
    PacketList packet_list{};
    packet_list.deserialize(buf);
    auto &packets_records = packet_list.packet_records;

    // Choose random packet to mutate
    std::minstd_rand rnd(seed);
    auto &packet_record = packets_records[rnd() % packets_records.size()];
    auto &packet = packet_record.packet_data;

    // Compute how large the packet may grow while still satisfying the max_size constraint
    size_t max_packet_grow = max_size - size;
    size_t max_packet_size = packet.size() + max_packet_grow;

    // Allocate some extra buffer space for insert mutations
    size_t old_packet_size = packet.size();
    if (max_packet_size > packet.size())
        packet.resize(max_packet_size);

    // The packet might be smaller/larger due to remove/insert mutations
    size_t new_packet_size = LLVMFuzzerMutate(packet.data(), old_packet_size, max_packet_size);
    packet_record.packet_header.packet_length = new_packet_size;

    // Serialize into buffer
    std::vector<uint8_t> new_data;
    packet_list.serialize(new_data);

    if (new_data.empty())
        return 0;

    assert(new_data.size() <= max_size);
    std::copy(new_data.begin(), new_data.end(), data);

    return new_data.size();
}
