#include "matrix.h"

namespace tomsolver {

namespace internal {} // namespace internal

Mat::Mat(int rows, int cols) noexcept {
    assert(rows > 0 && cols > 0);
    data.resize(rows);
    for (auto &row : data) {
        row.resize(cols);
    }
}

bool Mat::Empty() const noexcept {
    return data.empty();
}

int Mat::Rows() const noexcept {
    return data.size();
}

int Mat::Cols() const noexcept {
    if (Rows()) {
        return data[0].size();
    }
    return 0;
}

std::string Mat::ToString() const noexcept {
    if (Empty())
        return "[]";
    std::string s;
    s.reserve(256);

    auto OutputRow = [&](int i) {
        int j = 0;
        for (; j < Cols() - 1; ++j) {
            s += data[i][j]->ToString() + ", ";
        }
        s += data[i][j]->ToString();
    };

    s += "[";
    OutputRow(0);
    s += "\n";

    int i = 1;
    for (; i < Rows() - 1; ++i) {
        s += " ";
        OutputRow(i);
        s += "\n";
    }
    s += " ";
    OutputRow(i);
    s += "]";
    return s;
}

Vec::Vec(const std::initializer_list<Node> &lst) noexcept : Mat(lst.size(), 1) {
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        auto &node = const_cast<Node &>(*it);
        data[it - lst.begin()][0] = std::move(node);
    }
}

} // namespace tomsolver
