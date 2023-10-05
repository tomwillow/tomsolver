#include "symmat.h"
#include "functions.h"
#include "linear.h"
#include "config.h"

#include "memory_leak_detection.h"

#include <gtest/gtest.h>

#include <iostream>

using namespace std;
using namespace tomsolver;

// Armijo方法一维搜索，寻找alpha
double Armijo(const Vec &x, const Vec &d, function<Vec(Vec)> f, function<Mat(Vec)> df, double uncert = 1e-5) {
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

TEST(Solve, FindAlphaByArmijo) {
    MemoryLeakDetection mld;

    GetConfig().epsilon = 1e-6;

    auto g = [](const Vec &x) -> Vec {
        return {pow(x[0] - 4, 4) + pow(x[1] - 3, 2) + 4 * pow(x[2] + 5, 4)};
    };

    auto dg = [](const Vec &x) -> Vec {
        return {4 * pow(x[0] - 4, 3), 2 * (x[1] - 3), 16 * pow(x[2] + 5, 3)};
    };

    Vec x{4, 2, -1};
    Vec d = -Vec{0, -2, 1024};
    double alpha = Armijo(x, d, g, dg);
    cout << alpha << endl;
    double expected = 0.003866;
}

// 割线法 进行一维搜索，寻找alpha
double FindAlpha(const Vec &x, const Vec &d, function<Vec(Vec)> f, double uncert = 1e-5) {
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

TEST(Solve, FindAlpha) {

    auto g = [](const Vec &x) -> Vec {
        return {pow(x[0] - 4, 4), pow(x[1] - 3, 2), 4 * pow(x[2] + 5, 4)};
    };

    Vec x{4, 2, -1};
    Vec d = -Vec{0, -2, 1024};
    double alpha = FindAlpha(x, d, g);
    cout << alpha << endl;

    // FIXME: not match got results
    double expected = 0.003866;
}

Vec Solve(const std::unordered_map<std::string, double> &varsTable, const SymVec &equations) {
    const int max_iterator = 100; // 最大迭代次数限制
    int it = 0;                   // 迭代计数
    auto table = varsTable;
    int n = table.size(); // 未知量数量
    Vec q(n);             // x向量
    bool doPrint = true;

    std::vector<string> vars(n);
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

            if (doPrint) {
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
    if (doPrint) {
        cout << "success" << endl;
    }
    return q;

overIterate:
    throw runtime_error("迭代次数超出限制");
}

TEST(Solve, Base) {
    MemoryLeakDetection mld;

    std::setlocale(LC_ALL, ".UTF8");

    Node f1 = Var("a") * cos(Var("x1")) + Var("b") * cos(Var("x1") - Var("x2")) +
              Var("c") * cos(Var("x1") - Var("x2") - Var("x3"));
    Node f2 = Var("a") * sin(Var("x1")) + Var("b") * sin(Var("x1") - Var("x2")) +
              Var("c") * sin(Var("x1") - Var("x2") - Var("x3"));
    Node f3 = Var("x1") - Var("x2") - Var("x3");

    std::vector<std::string> vars = {"x1", "x2", "x3"};
    SymVec equations{Clone(f1), Clone(f2), Clone(f3)};
    equations.Subs({{"a", 0.425}, {"b", 0.39243}, {"c", 0.109}});
    cout << equations.ToString() << endl;

    SymVec b{Num(0.5), Num(0.4), Num(0)};
    SymVec n = equations - b;

    SymMat ja = Jacobian(n, vars);
    cout << ja.ToString() << endl;

    std::unordered_map<std::string, double> varsTable = {{"x1", 1}, {"x2", 1}, {"x3", 1}};

    Vec n0 = n.Clone().Subs(varsTable).Calc().ToMat().ToVec();
    cout << "n0 = " << n0 << endl;

    Mat ja0 = ja.Clone().Subs(varsTable).Calc().ToMat();
    cout << "ja0 = " << ja0 << endl;

    // Vec x = SolveLinear(ja0, n0);
    // cout << "x = " << x << endl;

    Vec ret = Solve(varsTable, n);
    cout << "ret = " << ret << endl;

    Vec expected{{1.5722855035930956, 1.6360330989069252, -0.0637475947386077}};
    ASSERT_EQ(ret, expected);
}
