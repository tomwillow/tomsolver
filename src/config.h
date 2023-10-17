#pragma once
#include <string>

namespace tomsolver {

enum class LogLevel { OFF, FATAL, ERROR, WARN, INFO, DEBUG, TRACE, ALL };

enum class NonlinearMethod { NEWTON_RAPHSON, LM };

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

    /**
     * 求解方法
     */
    NonlinearMethod nonlinearMethod = NonlinearMethod::NEWTON_RAPHSON;

    /**
     * 非线性方程求解时，当没有为VarsTable传初值时，设定的初值
     */
    double initialValue = 1.0;

    Config();

    void Reset() noexcept;

    const char *GetDoubleFormatStr() const noexcept;

private:
    char doubleFormatStr[16] = "%.16f";
};

std::string ToString(double value) noexcept;

Config &GetConfig() noexcept;

} // namespace tomsolver