#include "predictor.h"

#include <stdexcept>

HawkeyePredictor::HawkeyePredictor(std::size_t num_entries,
                                   int counter_bits)
    : num_entries_(num_entries),
      counter_bits_(counter_bits),
      max_counter_((1 << counter_bits) - 1),
      initial_counter_(1 << (counter_bits - 1)),
      counters_(num_entries, initial_counter_)
{
    if (num_entries_ == 0)
        throw std::invalid_argument("Predictor must have at least one entry");

    if (counter_bits_ <= 0 || counter_bits_ >= 31)
        throw std::invalid_argument("Invalid counter width");
}

std::size_t HawkeyePredictor::index(uint64_t pc) const
{
    uint64_t hash = pc ^ (pc >> 13) ^ (pc >> 26);

    return static_cast<std::size_t>(hash % num_entries_);
}

void HawkeyePredictor::train(uint64_t pc, bool opt_hit)
{
    std::size_t idx = index(pc);

    if (opt_hit) {
        if (counters_[idx] < max_counter_)
            ++counters_[idx];
    }
    else {
        if (counters_[idx] > 0)
            --counters_[idx];
    }
}

bool HawkeyePredictor::predict(uint64_t pc) const
{
    const int counter = counters_[index(pc)];

    return (counter & (1 << (counter_bits_ - 1))) != 0;
}

int HawkeyePredictor::get_counter(uint64_t pc) const
{
    return counters_[index(pc)];
}
