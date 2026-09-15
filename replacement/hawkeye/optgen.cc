#include "optgen.h"

#include <stdexcept>

OPTgen::OPTgen(std::size_t num_sets,
               std::size_t associativity,
               std::size_t history_multiplier)
    : num_sets_(num_sets),
      associativity_(associativity),
      history_length_(associativity * history_multiplier),
      history_(num_sets)
{
}

bool OPTgen::access(std::size_t set_idx, uint64_t address)
{
    if (set_idx >= num_sets_)
        throw std::out_of_range("OPTgen set index out of range");

    auto& history = history_[set_idx];

    std::size_t previous = history.size();

    for (std::size_t i = history.size(); i-- > 0;) {
        if (history[i].address == address) {
            previous = i;
            break;
        }
    }

    bool opt_hit = false;

    if (previous != history.size()) {

        opt_hit = true;

        for (std::size_t i = previous; i < history.size(); ++i) {
            if (history[i].occupancy >= associativity_) {
                opt_hit = false;
                break;
            }
        }

        if (opt_hit) {
            for (std::size_t i = previous; i < history.size(); ++i)
                ++history[i].occupancy;
        }
    }

    history.push_back({address, 0});

    if (history.size() > history_length_)
        history.pop_front();

    return opt_hit;
}
