#include <tomsolver/tomsolver.h>

using namespace tomsolver;

int main() {
    // set the locale output to utf-8
    std::setlocale(LC_ALL, ".UTF8");

    try {
        // create equations from string
        SymVec f = {
            "x^2+y^2-25"_f,
            "x^2-y^2-7"_f,
        };

        Config::Get().logLevel = LogLevel::TRACE;

        VarsTable initialValues{{"x", 5}, {"y", -5}};

        // solve it with custom initial values
        VarsTable ans = Solve(f, initialValues);

        // print the solution
        std::cout << ans << std::endl;

        // get the values of solution
        std::cout << "x = " << ans["x"] << std::endl;
        std::cout << "y = " << ans["y"] << std::endl;

        // substitute the obtained variables into the equations to verify the solution
        // if the result is 0, it indicates that the solution is correct
        std::cout << "equations: " << f.Subs(ans).Calc() << std::endl;
    } catch (const std::runtime_error &err) {
        // if any error occurs, exception will be thrown
        std::cerr << err.what() << std::endl;
        return -1;
    }

    return 0;
}