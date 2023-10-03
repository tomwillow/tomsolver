#include "subs.h"

#include <unordered_map>

namespace tomsolver {

namespace internal {

class SubsFunctions {
public:
    static void SubsInner(Node &node, const std::unordered_map<std::string, Node> &dict) noexcept {
        // 前序遍历。非递归实现。

        std::stack<NodeImpl *> stk;

        auto Replace = [&dict](Node &cur) -> bool {
            if (cur->type != NodeType::VARIABLE) {
                return false;
            }
            const auto &itor = dict.find(cur->varname);
            if (itor == dict.end()) {
                return false;
            }

            Node cloned = std::move(Clone(itor->second));
            cloned->parent = cur->parent;
            cur = std::move(cloned);
            return true;
        };

        if (Replace(node)) {
            return;
        }

        if (node->right) {
            if (!Replace(node->right))
                stk.push(node->right.get());
        }
        if (node->left) {
            if (!Replace(node->left))
                stk.push(node->left.get());
        }
        while (!stk.empty()) {
            NodeImpl *f = stk.top();
            stk.pop();

            if (f->right) {
                if (!Replace(f->right))
                    stk.push(f->right.get());
            }
            if (f->left) {
                if (!Replace(f->left))
                    stk.push(f->left.get());
            }
        }
    }
};

} // namespace internal

Node Subs(const Node &node, const std::string &oldVar, const Node &newNode) noexcept {
    Node node2 = Clone(node);
    return Subs(std::move(node2), oldVar, newNode);
}

Node Subs(Node &&node, const std::string &oldVar, const Node &newNode) noexcept {
    Node ret = std::move(node);
    std::unordered_map<std::string, Node> dict;
    dict.insert({oldVar, Clone(newNode)});
    internal::SubsFunctions::SubsInner(ret, dict);
#ifndef NDEBUG
    ret->CheckParent();
#endif
    return ret;
}

Node Subs(const Node &node, const std::vector<std::string> &oldVars, const Vec &newNodes) noexcept {
    Node node2 = Clone(node);
    return Subs(std::move(node2), oldVars, newNodes);
}

Node Subs(Node &&node, const std::vector<std::string> &oldVars, const Vec &newNodes) noexcept {
    assert(oldVars.size() == newNodes.Rows());
    Node ret = std::move(node);
    std::unordered_map<std::string, Node> dict;
    for (size_t i = 0; i < oldVars.size(); ++i) {
        dict.insert({oldVars[i], Clone(newNodes[i])});
    }
    internal::SubsFunctions::SubsInner(ret, dict);
#ifndef NDEBUG
    ret->CheckParent();
#endif
    return ret;
}

} // namespace tomsolver
