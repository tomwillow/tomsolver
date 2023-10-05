#include "nonlinear.h"

#include "linear.h"
#include "error_type.h"
#include "config.h"

#include <cassert>

using std::cout;
using std::endl;
using std::runtime_error;

namespace tomsolver {

double Armijo(const Vec &x, const Vec &d, std::function<Vec(Vec)> f, std::function<Mat(Vec)> df, double uncert) {
    double alpha = 1;   // a > 0
    double gamma = 0.4; // 取值范围(0, 0.5)越大越快
    double sigma = 0.5; // 取值范围(0, 1)越大越慢
    Vec x_new(x);
    while (1) {
        x_new = x + alpha * d;

        auto l = f(x_new).Norm2();
        auto r = (f(x).AsMat() + gamma * alpha * df(x).Transpose() * d).Norm2();
        if (l <= r) //检验条件
        {
            break;
        } else
            alpha = alpha * sigma; // 缩小alpha，进入下一次循环
    }
    return alpha;
}

double FindAlpha(const Vec &x, const Vec &d, std::function<Vec(Vec)> f, double uncert) {
    double alpha_cur = 0;

    double alpha_new = 1;

    int it = 0;
    int maxIter = 100;

    Vec g_cur = f(x + alpha_cur * d);

    while (abs(alpha_new - alpha_cur) > alpha_cur * uncert) {
        double alpha_old = alpha_cur;
        alpha_cur = alpha_new;
        Vec g_old = g_cur;
        g_cur = f(x + alpha_cur * d);

        if (g_cur < g_old) {
            break;
        }

        // FIXME: nan occurred
        alpha_new = EachDivide((g_cur * alpha_old - g_old * alpha_cur), (g_cur - g_old)).NormNegInfinity();

        // cout << it<<"\t"<<alpha_new << endl;
        if (it++ > maxIter) {
            cout << "FindAlpha: over iterator" << endl;
            break;
        }
    }
    return alpha_new;
}

Vec Solve(const std::unordered_map<std::string, double> &varsTable, const SymVec &equations) {
    const int max_iterator = 100; // 最大迭代次数限制
    int it = 0;                   // 迭代计数
    auto table = varsTable;
    int n = table.size(); // 未知量数量
    Vec q(n);             // x向量

    std::vector<std::string> vars(n);
    int index = 0;
    for (auto &pr : table) {
        vars[index] = pr.first;
        q[index] = pr.second;
        ++index;
    }

    SymMat JaEqs = Jacobian(equations, vars);
    while (1) {

        double mu = 1e-5; // LM方法的λ值

        Vec F = equations.Clone().Subs(table).Calc().ToMat().ToVec(); // 计算F

        if (F == 0) // F值为0，满足方程组求根条件
            goto success;

        Vec FNew(n);   // 下一轮F
        Vec deltaq(n); // Δq
        while (1) {

            Mat J = JaEqs.Clone().Subs(table).Calc().ToMat(); // 计算雅可比矩阵

            // 说明：
            // 标准的LM方法中，d=-(J'*J+λI)^(-1)*J'F，其中J'*J是为了确保矩阵对称正定。但实践发现此时的d过大，很难收敛。
            // 所以只用了牛顿法的 d=-(J+λI)^(-1)*F

            // enumError err = SolveLinear(J.Transpose()*J + mu * Matrix(J.rows, J.cols).Ones(), d, -J.Transpose()*F);

            // 方向向量
            Vec d = SolveLinear(J + mu * Mat(J.Rows(), J.Cols()).Ones(), -F); // 得到d

            double alpha = Armijo(
                q, d,
                [&](Vec v) -> Vec {
                    int i = 0;
                    for (auto &var : vars) {
                        table[var] = v[i++];
                    }
                    return equations.Clone().Subs(table).Calc().ToMat().ToVec();
                },
                [&](Vec v) -> Mat {
                    int i = 0;
                    for (auto &var : vars) {
                        table[var] = v[i++];
                    }
                    return JaEqs.Clone().Subs(table).Calc().ToMat();
                }); // 进行1维搜索得到alpha

            // double alpha = FindAlpha(q, d, std::bind(SixBarAngPosition, std::placeholders::_1, thetaCDKL, Hhit));

            // for (size_t i = 0; i < alpha.rows; ++i)
            //{
            //	if (alpha[i] != alpha[i])
            //		alpha[i] = 1.0;
            //}

            deltaq = alpha * d; // 计算Δq

            Vec qTemp = q + deltaq;
            int i = 0;
            for (auto &var : vars) {
                table[var] = qTemp[i++];
            }

            FNew = equations.Clone().Subs(table).Calc().ToMat().ToVec(); // 计算新的F

            if (GetConfig().logLevel >= LogLevel::TRACE) {
                cout << "it=" << it << endl;
                cout << "\talpha=" << alpha << endl;
                cout << "mu=" << mu << endl;
                cout << "F.Norm2()=" << F.Norm2() << endl;
                cout << "FNew.Norm2()=" << FNew.Norm2() << endl;
                cout << "\tF(x k+1).Norm2()\t" << ((FNew.Norm2() < F.Norm2()) ? "<" : ">=") << "\tF(x k).Norm2()\t"
                     << endl;
            }

            if (FNew.Norm2() < F.Norm2()) // 满足下降条件，跳出内层循环
            {
                break;
            } else {
                mu *= 10.0; // 扩大λ，使模型倾向梯度下降方向
            }

            if (it++ == max_iterator)
                goto overIterate;
        }

        q += deltaq; // 应用Δq，更新q值

        int i = 0;
        for (auto &var : vars) {
            table[var] = q[i++];
        }

        F = FNew; // 更新F

        if (it++ == max_iterator)
            goto overIterate;
    }

success:
    if (GetConfig().logLevel >= LogLevel::TRACE) {
        cout << "success" << endl;
    }
    return q;

overIterate:
    throw runtime_error("迭代次数超出限制");
}

} // namespace tomsolver