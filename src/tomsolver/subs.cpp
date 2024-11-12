#include "subs.h"
#include "node.h"

#include <functional>
#include <map>
#include <tuple>

namespace tomsolver {

namespace internal {

class SubsFunctions {
public:
    // 前序遍历。非递归实现。
    static Node SubsInner(Node node, const std::map<std::string, Node> &dict) noexcept {

        std::stack<std::reference_wrapper<NodeImpl>> stk;

        auto Replace = [&dict](Node &cur) {
            if (cur->type != NodeType::VARIABLE) {
                return false;
            }

            auto itor = dict.find(cur->varname);
            if (itor == dict.end()) {
                return false;
            }

            auto parent = cur->parent;
            cur = Clone(itor->second);
            cur->parent = parent;

            return true;
        };

        if (!Replace(node)) {
            auto TryReplace = [&stk, &Replace](Node &cur) {
                if (cur && !Replace(cur)) {
                    stk.emplace(*cur);
                }
            };

            TryReplace(node->right);
            TryReplace(node->left);

            while (!stk.empty()) {
                auto &f = stk.top().get();
                stk.pop();
                TryReplace(f.right);
                TryReplace(f.left);
            }
        }

#ifndef NDEBUG
        node->CheckParent();
#endif
        return node;
    }
};

} // namespace internal

Node Subs(const Node &node, const std::string &oldVar, const Node &newNode) noexcept {
    return Subs(Clone(node), oldVar, newNode);
}

Node Subs(Node &&node, const std::string &oldVar, const Node &newNode) noexcept {
    std::map<std::string, Node> dict;
    dict.insert({oldVar, Clone(newNode)});
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

Node Subs(const Node &node, const std::vector<std::string> &oldVars, const SymVec &newNodes) noexcept {
    return Subs(Clone(node), oldVars, newNodes);
}

Node Subs(Node &&node, const std::vector<std::string> &oldVars, const SymVec &newNodes) noexcept {
    assert(static_cast<int>(oldVars.size()) == newNodes.Rows());
    std::map<std::string, Node> dict;
    for (size_t i = 0; i < oldVars.size(); ++i) {
        dict.insert({oldVars[i], Clone(newNodes[i])});
    }
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

Node Subs(const Node &node, const std::map<std::string, Node> &dict) noexcept {
    return Subs(Clone(node), dict);
}

Node Subs(Node &&node, const std::map<std::string, Node> &dict) noexcept {
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

Node Subs(const Node &node, const std::map<std::string, double> &varValues) noexcept {
    return Subs(Clone(node), varValues);
}

Node Subs(Node &&node, const std::map<std::string, double> &varValues) noexcept {
    std::map<std::string, Node> dict;
    for (auto &item : varValues) {
        dict.insert({item.first, Num(item.second)});
    }
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

Node Subs(const Node &node, const VarsTable &varsTable) noexcept {
    return Subs(Clone(node), varsTable);
}

Node Subs(Node &&node, const VarsTable &varsTable) noexcept {
    std::map<std::string, Node> dict;
    for (auto &item : varsTable) {
        dict.insert({item.first, Num(item.second)});
    }
    return internal::SubsFunctions::SubsInner(Move(node), dict);
}

} // namespace tomsolver
