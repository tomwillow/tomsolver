#include "diff.h"

#include <queue>
#include "subs.h"

namespace tomsolver {

namespace internal {

class SubsFunctions {
public:
    static void SubsInner(Node &node, const std::string &oldVar, const Node &newNode) noexcept {
        // 前序遍历。非递归实现。

        std::stack<NodeImpl *> stk;

        auto Replace = [&oldVar, &newNode](Node &cur) -> bool {
            if (cur->type == NodeType::VARIABLE && cur->varname == oldVar) {
                Node cloned = std::move(Clone(newNode));
                cloned->parent = cur->parent;
                cur = std::move(cloned);
                return true;
            }
            return false;
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
    Node n = std::move(node);
    internal::SubsFunctions ::SubsInner(n, oldVar, newNode);
#ifndef NDEBUG
    n->CheckParent();
#endif
    return n;
}

} // namespace tomsolver
