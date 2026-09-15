#ifndef HAWKEYE_PREDICTOR_H
#define HAWKEYE_PREDICTOR_H

#include <cstddef>
#include <cstdint>
#include <vector>

class HawkeyePredictor
{
    std::size_t num_entries_;
    int counter_bits_;
    int max_counter_;
    int initial_counter_;

    std::vector<int> counters_;

    std::size_t index(uint64_t pc) const;

public:
    HawkeyePredictor(std::size_t num_entries = 8192,
                     int counter_bits = 3);

    void train(uint64_t pc, bool opt_hit);

    bool predict(uint64_t pc) const;

    int get_counter(uint64_t pc) const;
};

#endif
