#pragma once

#include "error_type.h"
#include "mat.h"

#include <vector>
#include <unordered_map>
#include <iostream>

namespace tomsolver {

/**
 * 变量表。
 * 内部保存了多个变量名及其数值的对应关系。
 */
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

    /**
     * 变量数量。
     */
    int VarNums() const noexcept;

    /**
     * 返回std::vector容器包装的变量名数组。
     */
    const std::vector<std::string> &Vars() const noexcept;

    /**
     * 返回所有变量名对应的值的数值向量。
     */
    const Vec &Values() const noexcept;

    /**
     * 设置数值向量。
     */
    void SetValues(const Vec &v) noexcept;

    std::unordered_map<std::string, double>::const_iterator begin() const noexcept;

    std::unordered_map<std::string, double>::const_iterator end() const noexcept;

    std::unordered_map<std::string, double>::const_iterator cbegin() const noexcept;

    std::unordered_map<std::string, double>::const_iterator cend() const noexcept;

    bool operator==(const VarsTable &rhs) const noexcept;

    /**
     * 根据变量名获取数值。
     * @exception out_of_range 如果没有这个变量，抛出异常
     */
    double operator[](const std::string &varname) const;

private:
    std::vector<std::string> vars;
    Vec values;
    std::unordered_map<std::string, double> table;
};

std::ostream &operator<<(std::ostream &out, const VarsTable &table) noexcept;

} // namespace tomsolver