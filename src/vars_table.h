#pragma once

#include "error_type.h"
#include "mat.h"

#include <vector>
#include <unordered_map>
#include <iostream>

namespace tomsolver {

class VarsTable {
public:
    /**
     * 新建变量表。
     * @param vars 变量数组
     * @param initValue 初值
     */
    VarsTable(const std::vector<std::string> &vars, double initValue);

    /**
     * 新建变量表。
     * @param vars 变量数组
     * @param initValue 初值
     */
    explicit VarsTable(const std::initializer_list<std::pair<std::string, double>> &initList);

    /**
     * 新建变量表。
     * @param vars 变量数组
     * @param initValue 初值
     */
    explicit VarsTable(const std::unordered_map<std::string, double> &table) noexcept;

    int VarNums() const noexcept;

    const std::vector<std::string> &Vars() const noexcept;

    const Vec &Values() const noexcept;

    void SetValues(const Vec &v) noexcept;

    std::unordered_map<std::string, double>::iterator begin() noexcept {
        return table.begin();
    }

    std::unordered_map<std::string, double>::iterator end() noexcept {
        return table.end();
    }

    std::unordered_map<std::string, double>::const_iterator begin() const noexcept {
        return table.begin();
    }

    std::unordered_map<std::string, double>::const_iterator end() const noexcept {
        return table.end();
    }

    std::unordered_map<std::string, double>::const_iterator cbegin() const noexcept {
        return table.cbegin();
    }

    std::unordered_map<std::string, double>::const_iterator cend() const noexcept {
        return table.cend();
    }

    bool operator==(const VarsTable &rhs) const noexcept;

private:
    std::vector<std::string> vars;
    Vec values;
    std::unordered_map<std::string, double> table;
};

std::ostream &operator<<(std::ostream &out, const VarsTable &table) noexcept;

} // namespace tomsolver