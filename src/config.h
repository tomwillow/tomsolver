#pragma once
#include <string>

namespace tomsolver {

struct Config {
    /**
     * 指定出现浮点数无效值(inf, -inf, nan)时，是否抛出异常。默认为true。
     */
    bool throwOnInvalidValue = true;

    Config();

    void Reset() noexcept;

    const char *GetDoubleFormatStr() const noexcept;

private:
    char doubleFormatStr[16] = "%.16f";
};

std::string ToString(double value) noexcept;

Config &GetConfig() noexcept;

} // namespace tomsolver