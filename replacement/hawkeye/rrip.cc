#include "rrip.h"

#include <algorithm>
#include <stdexcept>

namespace
{
constexpr int MAX_RRPV = 7;
constexpr int FRIENDLY_MAX_RRPV = 6;
}

void update_rrpv(std::vector<int>& rrpv,
                 std::size_t way,
                 Classification cls,
                 bool is_hit)
{
    if (way >= rrpv.size())
        throw std::out_of_range("RRIP way out of range");

    if (cls == Classification::CACHE_AVERSE) {
        rrpv[way] = MAX_RRPV;
        return;
    }

    if (is_hit) {
        rrpv[way] = 0;
        return;
    }

    for (std::size_t i = 0; i < rrpv.size(); ++i) {
        if (i != way && rrpv[i] < FRIENDLY_MAX_RRPV)
            ++rrpv[i];
    }

    rrpv[way] = 0;
}

std::size_t find_victim(std::vector<int>& rrpv)
{
    if (rrpv.empty())
        throw std::invalid_argument("RRIP vector cannot be empty");

    while (true) {

        auto victim =
            std::find(rrpv.begin(), rrpv.end(), MAX_RRPV);

        if (victim != rrpv.end())
            return static_cast<std::size_t>(
                std::distance(rrpv.begin(), victim));

        for (auto& value : rrpv) {
            if (value < MAX_RRPV)
                ++value;
        }
    }
}
