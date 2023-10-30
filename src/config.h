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

    /**
     * 是否允许不定方程存在。
     * 例如，当等式数量大于未知数数量时，方程组成为不定方程；
     * 如果允许，此时将返回一组特解；如果不允许，将抛出异常。
     */
    bool allowIndeterminateEquation = false;

    void Reset() noexcept;

    template <auto = 0>
    static Config &get() {
        static Config config;
        return config;
    }
};

std::string ToString(double value) noexcept;

} // namespace tomsolver