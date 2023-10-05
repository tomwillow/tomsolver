#pragma once
#include <string>

namespace tomsolver {

enum class LogLevel { OFF, FATAL, ERROR, WARN, INFO, DEBUG, TRACE, ALL };

struct Config {
    /**
     * 指定出现浮点数无效值(inf, -inf, nan)时，是否抛出异常。默认为true。
     */
    bool throwOnInvalidValue = true;

    double epsilon = 1.0e-9;

    LogLevel logLevel = LogLevel::WARN;

    /**
     * 最大迭代次数限制
     */
    int maxIterations = 100;

    Config();

    void Reset() noexcept;

    const char *GetDoubleFormatStr() const noexcept;

private:
    char doubleFormatStr[16] = "%.16f";
};

std::string ToString(double value) noexcept;

Config &GetConfig() noexcept;

} // namespace tomsolver