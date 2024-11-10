#include <tomsolver/tomsolver.hpp>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using namespace tomsolver;

const char usage[] = "=========== diff machine ===========\n"
                     "example: \n"
                     ">>x^5\n"
                     "ans = \n"
                     "  5*x^4\n"
                     "\n"
                     "optional functions:\n"
                     "sin(x) cos(x) tan(x) asin(x) acos(x) atan(x)\n"
                     "sqrt(x) log(x) log2(x) log10(x) exp(x)\n"
                     "====================================\n";

int main() {
    cout << usage << endl;

    while (1) {
        // let user input an expression
        cout << ">>";
        std::string expr;
        cin >> expr;

        try {
            // parse to be expression tree
            Node node = Parse(expr);

            // analyze variables
            auto varnamesSet = node->GetAllVarNames();
            std::vector<std::string> varnames(varnamesSet.begin(), varnamesSet.end());
            VarsTable varsTable(varnames, 0);
            std::string varname;

            if (varsTable.Vars().size() == 1) {
                // only one variable
                varname = varsTable.Vars()[0];
            } else if (varsTable.Vars().size() > 1) {
                // multiple variables
                cout << "more than 1 variable. who do you want to differentiate?" << endl;
                cout << ">>";
                cin >> varname;
                if (!varsTable.Has(varname)) {
                    throw std::runtime_error("no variable \"" + varname + "\" in expression: " + expr);
                }
            }

            Node dnode = Diff(Move(node), varname);
            cout << "ans = " << endl;
            cout << "  " << dnode->ToString() << endl;
        } catch (const std::runtime_error &err) { cerr << err.what() << endl; }
    }

    return 0;
}