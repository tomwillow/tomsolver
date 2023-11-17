#include "config.h"

#include <array>
#include <cstdio>
#include <regex>
#include <tuple>

namespace tomsolver {

std::string ToString(double value) noexcept {
    static const std::array strategies = {
        std::tuple{"%.16e", std::regex{"\\.?0+(?=e)"}},
        std::tuple{"%.16f", std::regex{"\\.?0+(?=$)"}},
    };

    if (value == 0.0) {
        return "0";
    }

    char buf[64];

    // 绝对值过大 或者 绝对值过小，应该使用科学计数法来表示
    auto getStrategyIdx = [absValue = std::abs(value)] {
        return (absValue >= 1.0e16 || absValue <= 1.0e-16) ? 0 : 1;
    };

    auto &strategy = strategies[getStrategyIdx()];
    auto fmt = std::get<0>(strategy);
    auto &re = std::get<1>(strategy);

#ifdef WIN32
    sprintf_s(buf, fmt, value);
#else
    sprintf(buf, fmt, value);
#endif
    return std::regex_replace(buf, re, "");
}

void Config::Reset() noexcept {
    *this = {};
}

} // namespace tomsolver