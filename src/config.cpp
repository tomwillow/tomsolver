#include "config.h"

#include <array>
#include <cstdio>
#include <regex>
#include <tuple>

namespace tomsolver {

namespace {

static const std::tuple<const char *, std::regex> strategies[] = {
    {"%.16e", std::regex{"\\.?0+(?=e)"}},
    {"%.16f", std::regex{"\\.?0+(?=$)"}},
};

}

std::string ToString(double value) noexcept {
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

    snprintf(buf, sizeof(buf), fmt, value);
    return std::regex_replace(buf, re, "");
}

void Config::Reset() noexcept {
    *this = {};
}

Config &Config::Get() {
    static Config config;
    return config;
}

} // namespace tomsolver