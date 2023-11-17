#include "vars_table.h"

#include "config.h"

#include <algorithm>
#include <cassert>

namespace tomsolver {

VarsTable::VarsTable(const std::vector<std::string> &vars, double initValue)
    : vars(vars), values(static_cast<int>(vars.size()), initValue) {
    for (auto &var : vars) {
        table.try_emplace(var, initValue);
    }
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(std::initializer_list<std::pair<std::string, double>> initList)
    : VarsTable({initList.begin(), initList.end()}) {
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(const std::map<std::string, double> &table) noexcept
    : vars(table.size()), values(static_cast<int>(table.size())), table(table) {
    int i = 0;
    for (auto &item : table) {
        vars[i] = item.first;
        values[i] = item.second;
        ++i;
    }
}

int VarsTable::VarNums() const noexcept {
    return static_cast<int>(table.size());
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

bool VarsTable::Has(const std::string &varname) const noexcept {
    return table.find(varname) != table.end();
}

std::map<std::string, double>::const_iterator VarsTable::begin() const noexcept {
    return table.begin();
}

std::map<std::string, double>::const_iterator VarsTable::end() const noexcept {
    return table.end();
}

std::map<std::string, double>::const_iterator VarsTable::cbegin() const noexcept {
    return table.cbegin();
}

std::map<std::string, double>::const_iterator VarsTable::cend() const noexcept {
    return table.cend();
}

bool VarsTable::operator==(const VarsTable &rhs) const noexcept {
    return values.Rows() == rhs.values.Rows() &&
           std::equal(table.begin(), table.end(), rhs.table.begin(), [](const auto &lhs, const auto &rhs) {
               auto &lVar = lhs.first;
               auto &lVal = lhs.second;
               auto &rVar = rhs.first;
               auto &rVal = rhs.second;
               return lVar == rVar && std::abs(lVal - rVal) <= Config::get().epsilon;
           });
}

double VarsTable::operator[](const std::string &varname) const {
    auto it = table.find(varname);
    if (it == table.end()) {
        throw std::out_of_range("no such variable: " + varname);
    }
    return it->second;
}

std::ostream &operator<<(std::ostream &out, const VarsTable &table) noexcept {
    for (auto &item : table) {
        out << item.first << " = " << tomsolver::ToString(item.second) << std::endl;
    }
    return out;
}

} // namespace tomsolver