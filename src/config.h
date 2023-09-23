#pragma once
#include <string>

namespace tomsolver {

struct Config {
    bool throwOnInvalidValue = true;
    bool checkDomain = true;

    const char *GetDoubleFormatStr() const noexcept;

private:
    char doubleFormatStr[16] = "%.16f";
};

std::string ToString(double value) noexcept;

Config &GetConfig() noexcept;

} // namespace tomsolver