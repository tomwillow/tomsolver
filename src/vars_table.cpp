#include "vars_table.h"

#include <cassert>

namespace tomsolver {

VarsTable::VarsTable(const std::vector<std::string> &vars, double initValue)
    : vars(vars), values(vars.size(), initValue) {
    for (auto &var : vars) {
        table[var] = initValue;
    }
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(const std::initializer_list<std::pair<std::string, double>> &initList)
    : table(initList.begin(), initList.end()), values(initList.size()), vars(initList.size()) {
    int i = 0;
    for (auto &pr : initList) {
        vars[i] = pr.first;
        values[i] = pr.second;
        ++i;
    }
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(const std::unordered_map<std::string, double> &table) noexcept
    : table(table), values(table.size()), vars(table.size()) {
    int i = 0;
    for (auto &pr : table) {
        vars[i] = pr.first;
        values[i] = pr.second;
        ++i;
    }
}

int VarsTable::VarNums() const noexcept {
    return table.size();
}

const std::vector<std::string> &VarsTable::Vars() const noexcept {
    return vars;
}

const Vec &VarsTable::Values() const noexcept {
    return values;
}

void VarsTable::SetValues(const Vec &v) noexcept {
    assert(v.Rows() == values.Rows());
    values = v;
    for (int i = 0; i < values.Rows(); ++i) {
        table[vars[i]] = v[i];
    }
}

bool VarsTable::operator==(const VarsTable &rhs) const noexcept {
    int len = values.Rows();
    if (values.Rows() != rhs.values.Rows()) {
        return false;
    }

    // 这里会自动使用浮点数比较
    return values == rhs.values;
}

std::ostream &operator<<(std::ostream &out, const VarsTable &table) noexcept {
    for (auto &pr : table) {
        out << pr.first << " = " << pr.second << std::endl;
    }
    return out;
}

} // namespace tomsolver