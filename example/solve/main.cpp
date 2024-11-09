#include <tomsolver/tomsolver.hpp>

using namespace tomsolver;

int main() {
    /*
    Translate from Matlab code:

    root2d.m:
        function F = root2d(x)
            F(1) = exp(-exp(-(x(1)+x(2)))) - x(2)*(1+x(1)^2);
            F(2) = x(1)*cos(x(2)) + x(2)*sin(x(1)) - 0.5;
        end

    root2d_solve.m:
        format long
        fun = @root2d;
        x0 = [0,0];
        x = fsolve(fun,x0)

    result:
        x =

            0.353246561920553   0.606082026502285
     */

    // set the locale output to utf-8
    std::setlocale(LC_ALL, ".UTF8");

    try {
        // create equations from string
        SymVec f = {
            "exp(-exp(-(x1 + x2))) - x2 * (1 + x1 ^ 2)"_f,
            "x1 * cos(x2) + x2 * sin(x1) - 0.5"_f,
        };

        // set the initial value to be 0.0
        Config::Get().initialValue = 0.0;
        Config::Get().nonlinearMethod = NonlinearMethod::NEWTON_RAPHSON; // use the Newton-Raphson method
        Config::Get().allowIndeterminateEquation = true;

        // solve it!
        VarsTable ans = Solve(f);

        // print the solution
        std::cout << ans << std::endl;

        // get the values of solution
        std::cout << "x1 = " << ans["x1"] << std::endl;
        std::cout << "x2 = " << ans["x2"] << std::endl;

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