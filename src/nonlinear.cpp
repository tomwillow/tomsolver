#include "nonlinear.h"

#include "linear.h"
#include "error_type.h"
#include "config.h"

#include <cassert>

using std::cout;
using std::endl;
using std::runtime_error;

namespace tomsolver {

double Armijo(const Vec &x, const Vec &d, std::function<Vec(Vec)> f, std::function<Mat(Vec)> df) {
    double alpha = 1;   // a > 0
    double gamma = 0.4; // 取值范围(0, 0.5)越大越快
    double sigma = 0.5; // 取值范围(0, 1)越大越慢
    Vec x_new(x);
    while (1) {
        x_new = x + alpha * d;

        auto l = f(x_new).Norm2();
        auto r = (f(x).AsMat() + gamma * alpha * df(x).Transpose() * d).Norm2();
        if (l <= r) // 检验条件
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

    while (std::abs(alpha_new - alpha_cur) > alpha_cur * uncert) {
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

VarsTable SolveByNewtonRaphson(const VarsTable &varsTable, const SymVec &equations) {
    int it = 0; // 迭代计数
    VarsTable table = varsTable;
    int n = table.VarNums(); // 未知量数量
    Vec q(n);                // x向量

    SymMat jaEqs = Jacobian(equations, table.Vars());

    if (Config::get().logLevel >= LogLevel::TRACE) {
        cout << "Jacobian = " << jaEqs.ToString() << endl;
    }

    while (1) {
        Vec phi = equations.Clone().Subs(table).Calc().ToMat().ToVec();
        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << "iteration = " << it << endl;
            cout << "phi = " << phi << endl;
        }

        if (phi == 0) {
            break;
        }

        if (it > Config::get().maxIterations) {
            throw runtime_error("迭代次数超出限制");
        }

        Mat ja = jaEqs.Clone().Subs(table).Calc().ToMat();

        Vec deltaq = SolveLinear(ja, -phi);

        q += deltaq;

        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << "ja = " << ja << endl;
            cout << "deltaq = " << deltaq << endl;
            cout << "q = " << q << endl;
        }

        table.SetValues(q);

        ++it;
    }
    return table;
}

VarsTable SolveByLM(const VarsTable &varsTable, const SymVec &equations) {
    int it = 0; // 迭代计数
    VarsTable table = varsTable;
    int n = table.VarNums(); // 未知量数量
    Vec q = table.Values();  // x向量

    SymMat JaEqs = Jacobian(equations, table.Vars());

    if (Config::get().logLevel >= LogLevel::TRACE) {
        cout << "Jacobi = " << JaEqs << endl;
    }

    while (1) {
        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << "iteration = " << it << endl;
        }

        double mu = 1e-5; // LM方法的λ值

        Vec F = equations.Clone().Subs(table).Calc().ToMat().ToVec(); // 计算F

        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << "F = " << F << endl;
        }

        if (F == 0) // F值为0，满足方程组求根条件
            goto success;

        Vec FNew(n);   // 下一轮F
        Vec deltaq(n); // Δq
        while (1) {

            Mat J = JaEqs.Clone().Subs(table).Calc().ToMat(); // 计算雅可比矩阵

            if (Config::get().logLevel >= LogLevel::TRACE) {
                cout << "J = " << J << endl;
            }

            // 说明：
            // 标准的LM方法中，d=-(J'*J+λI)^(-1)*J'F，其中J'*J是为了确保矩阵对称正定。有时d会过大，很难收敛。
            // 牛顿法的 d=-(J+λI)^(-1)*F

            // 方向向量
            Vec d = SolveLinear(J.Transpose()*J + mu * Mat(J.Rows(), J.Cols()).Ones(), -(J.Transpose()*F).ToVec()); // 得到d

            if (Config::get().logLevel >= LogLevel::TRACE) {
                cout << "d = " << d << endl;
            }

            double alpha = Armijo(
                q, d,
                [&](Vec v) -> Vec {
                    table.SetValues(v);
                    return equations.Clone().Subs(table).Calc().ToMat().ToVec();
                },
                [&](Vec v) -> Mat {
                    table.SetValues(v);
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
            table.SetValues(qTemp);

            FNew = equations.Clone().Subs(table).Calc().ToMat().ToVec(); // 计算新的F

            if (Config::get().logLevel >= LogLevel::TRACE) {
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

            if (it++ == Config::get().maxIterations)
                goto overIterate;
        }

        q += deltaq; // 应用Δq，更新q值

        table.SetValues(q);

        F = FNew; // 更新F

        if (it++ == Config::get().maxIterations)
            goto overIterate;

        if (Config::get().logLevel >= LogLevel::TRACE) {
            cout << std::string(20, '=') << endl;
        }
    }

success:
    if (Config::get().logLevel >= LogLevel::TRACE) {
        cout << "success" << endl;
    }
    return table;

overIterate:
    throw runtime_error("迭代次数超出限制");
}

VarsTable Solve(const VarsTable &varsTable, const SymVec &equations) {
    switch (Config::get().nonlinearMethod) {
    case NonlinearMethod::NEWTON_RAPHSON:
        return SolveByNewtonRaphson(varsTable, equations);
    case NonlinearMethod::LM:
        return SolveByLM(varsTable, equations);
    }
    throw runtime_error("invalid config.NonlinearMethod value: " +
                        std::to_string(static_cast<int>(Config::get().nonlinearMethod)));
}

VarsTable Solve(const SymVec &equations) {
    auto varNames = equations.GetAllVarNames();
    std::vector<std::string> vecVarNames(varNames.begin(), varNames.end());
    VarsTable varsTable(std::move(vecVarNames), Config::get().initialValue);
    return Solve(varsTable, equations);
}

} // namespace tomsolver