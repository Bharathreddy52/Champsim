#include "hawkeye.h"
#include <algorithm>
#include <stdexcept>

namespace
{
constexpr std::size_t HISTORY_MULTIPLIER = 8;
constexpr int MAX_RRPV = 7;
}

hawkeye::hawkeye(CACHE* cache)
    : hawkeye(cache, cache->NUM_SET, cache->NUM_WAY)
{
}

hawkeye::hawkeye(CACHE* cache, long sets, long ways)
    : replacement(cache),
      NUM_SET(sets),
      NUM_WAY(ways),
      optgen(static_cast<std::size_t>(sets),
             static_cast<std::size_t>(ways),
             HISTORY_MULTIPLIER),
      predictor(),
      rrpv(static_cast<std::size_t>(sets),
           std::vector<int>(static_cast<std::size_t>(ways), MAX_RRPV)),
      line_pc(static_cast<std::size_t>(sets * ways), 0),
      history(static_cast<std::size_t>(sets))
{
}

void hawkeye::process_access(std::size_t set_idx,
                             uint64_t address,
                             uint64_t pc)
{
    auto& set_history = history.at(set_idx);

    uint64_t previous_pc = 0;
    bool found_previous = false;

    for (auto it = set_history.rbegin();
         it != set_history.rend();
         ++it)
    {
        if (it->address == address)
        {
            previous_pc = it->pc;
            found_previous = true;
            break;
        }
    }

    const bool opt_hit = optgen.access(set_idx, address);

    if (found_previous)
        predictor.train(previous_pc, opt_hit);

    set_history.push_back({address, pc});

    const std::size_t max_history =
        HISTORY_MULTIPLIER * static_cast<std::size_t>(NUM_WAY);

    if (set_history.size() > max_history)
        set_history.pop_front();
}

long hawkeye::find_victim(uint32_t triggering_cpu,
                          uint64_t instr_id,
                          long set,
                          const champsim::cache_block* current_set,
                          champsim::address ip,
                          champsim::address full_addr,
                          access_type type)
{
    (void)triggering_cpu;
    (void)instr_id;
    (void)current_set;
    (void)ip;
    (void)full_addr;
    (void)type;

    auto& set_rrpv = rrpv.at(static_cast<std::size_t>(set));

    return static_cast<long>(::find_victim(set_rrpv));
}

void hawkeye::replacement_cache_fill(uint32_t triggering_cpu,
                                     long set,
                                     long way,
                                     champsim::address full_addr,
                                     champsim::address ip,
                                     champsim::address victim_addr,
                                     access_type type)
{
    (void)triggering_cpu;
    (void)victim_addr;
    (void)type;

    const auto set_idx = static_cast<std::size_t>(set);
    const auto way_idx = static_cast<std::size_t>(way);

    const uint64_t address = full_addr.to<uint64_t>();
    const uint64_t pc = ip.to<uint64_t>();

    process_access(set_idx, address, pc);

    const bool cache_friendly = predictor.predict(pc);

    update_rrpv(
        rrpv.at(set_idx),
        way_idx,
        cache_friendly
            ? Classification::CACHE_FRIENDLY
            : Classification::CACHE_AVERSE,
        false);

    line_pc.at(
        set_idx * static_cast<std::size_t>(NUM_WAY) + way_idx) = pc;
}

void hawkeye::update_replacement_state(uint32_t triggering_cpu,
                                       long set,
                                       long way,
                                       champsim::address full_addr,
                                       champsim::address ip,
                                       champsim::address victim_addr,
                                       access_type type,
                                       uint8_t hit)
{
    (void)triggering_cpu;
    (void)victim_addr;
    (void)type;

    if (!hit)
        return;

    const auto set_idx = static_cast<std::size_t>(set);
    const auto way_idx = static_cast<std::size_t>(way);

    const uint64_t address = full_addr.to<uint64_t>();
    const uint64_t pc = ip.to<uint64_t>();

    process_access(set_idx, address, pc);

    const uint64_t stored_pc =
        line_pc.at(
            set_idx * static_cast<std::size_t>(NUM_WAY) + way_idx);

    const bool cache_friendly = predictor.predict(stored_pc);

    update_rrpv(
        rrpv.at(set_idx),
        way_idx,
        cache_friendly
            ? Classification::CACHE_FRIENDLY
            : Classification::CACHE_AVERSE,
        true);
}
