#ifndef HAWKEYE_OPTGEN_H
#define HAWKEYE_OPTGEN_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

class OPTgen
{
    struct Entry
    {
        uint64_t address;
        std::size_t occupancy;
    };

    std::size_t num_sets_;
    std::size_t associativity_;
    std::size_t history_length_;

    std::vector<std::deque<Entry>> history_;

public:
    OPTgen(std::size_t num_sets,
           std::size_t associativity,
           std::size_t history_multiplier = 8);

    bool access(std::size_t set_idx, uint64_t address);
};

#endif
