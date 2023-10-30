#include "vars_table.h"

#include "config.h"

#include <cassert>

namespace tomsolver {

VarsTable::VarsTable(const std::vector<std::string> &vars, double initValue)
    : vars(vars), values(static_cast<int>(vars.size()), initValue) {
    for (auto &var : vars) {
        table[var] = initValue;
    }
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(const std::initializer_list<std::pair<std::string, double>> &initList)
    : vars(initList.size()), values(static_cast<int>(initList.size())), table(initList.begin(), initList.end()) {
    int i = 0;
    for (auto &pr : initList) {
        vars[i] = pr.first;
        values[i] = pr.second;
        ++i;
    }
    assert(vars.size() == table.size() && "vars is not unique");
}

VarsTable::VarsTable(const std::map<std::string, double> &table) noexcept
    : vars(table.size()), values(static_cast<int>(table.size())), table(table) {
    int i = 0;
    for (auto &pr : table) {
        vars[i] = pr.first;
        values[i] = pr.second;
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
    if (values.Rows() != rhs.values.Rows()) {
        return false;
    }

    for (auto &pr : table) {
        const std::string &varname = pr.first;
        auto it = rhs.table.find(varname);
        if (it == rhs.table.end()) {
            return false;
        }
        double value = pr.second;
        if (std::abs(it->second - value) > Config::get().epsilon) {
            return false;
        }
    }
    return true;
}

double VarsTable::operator[](const std::string &varname) const {
    auto it = table.find(varname);
    if (it == table.end()) {
        throw std::out_of_range("no such variable: " + varname);
    }
    return it->second;
}

std::ostream &operator<<(std::ostream &out, const VarsTable &table) noexcept {
    for (auto &pr : table) {
        out << pr.first << " = " << tomsolver::ToString(pr.second) << std::endl;
    }
    return out;
}

} // namespace tomsolver