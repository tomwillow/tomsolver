#include <tomsolver/tomsolver.h>

using namespace tomsolver;

int main() {
    std::setlocale(LC_ALL, ".UTF8");

    try {
        // use the LM method
        Config::Get().nonlinearMethod = NonlinearMethod::LM;

        // define the equations
        // this is essentially a symbolic vector, and solving it means finding the roots where the vector equals the
        // zero vector
        SymVec f{
            Parse("a/(b^2)-c/(d^2)"),
            Parse("129.56108*b-(a/(b^2)+1/a-2*b/(a^2))"),
            Parse("129.56108*d-(d/(c^2)-c/(d^2)-1/a)"),
            Parse("5*e-7-(2/3*pi*a^2*b+((sqrt(3)*c^2)/(3*sqrt(c^2/3+d^2))+a-c)^2*pi*d^2/(c^2/3+d^2))"),
        };

        // substitude the symbolic constances "pi", "e" with their numerical values
        f.Subs(VarsTable{{"pi", PI}, {"e", std::exp(1.0)}});

        // print the f (symbolic vector)
        cout << f << endl;

        // solve
        auto ans = Solve(f);

        // print the solution
        cout << ans << endl;
    } catch (const std::runtime_error &err) {
        // if any error occurs, exception will be thrown
        std::cerr << err.what() << std::endl;
        return -1;
    }

    return 0;
}