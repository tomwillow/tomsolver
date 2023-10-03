#pragma once

#include "node.h"

namespace tomsolver {

class Mat {
public:
    /**
     *
     */
    Mat() noexcept {}

    /**
     *
     */
    Mat(int rows, int cols) noexcept;

    bool Empty() const noexcept;

    int Rows() const noexcept;

    int Cols() const noexcept;

    std::string ToString() const noexcept;

protected:
    std::vector<std::vector<Node>> data;
};

class Vec : public Mat {
public:
    /**
     * 使用初始化列表构造。注意列表内的对象将被强行移动至Vec内部。
     */
    Vec(const std::initializer_list<Node> &lst) noexcept;

    Node &operator[](std::size_t index) noexcept;

    const Node &operator[](std::size_t index) const noexcept;
};

} // namespace tomsolver
