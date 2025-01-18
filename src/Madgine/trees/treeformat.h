#pragma once

namespace Engine {

struct MADGINE_TREES_EXPORT FormatNode {

    FormatNode(void *id, FormatNode *parent = nullptr, size_t number = 1);

    FormatNode &addChild(void *id);

    void visitPreOrder(const std::function<void(void *, float, float)> &visitor);

    void visitPostOrder(const std::function<void(void *, float, float)> &visitor);

private:
    FormatNode *left_brother();

    FormatNode *get_lmost_sibling();

    FormatNode *left();

    FormatNode *right();

    FormatNode *ancestor(FormatNode &v, FormatNode *defaultAncestor);

    void execute_shifts();

private:
    friend struct FormatTreeBase;

    void *mId;

    float mX;
    float mY;

    std::list<FormatNode> mChildren;

    FormatNode *mParent = nullptr;
    FormatNode *mThread = nullptr;
    FormatNode *mAncestor = this;
    FormatNode *mLMostSibling = nullptr;

    size_t mNumber;
    float mMod = 0;
    float mChange = 0;
    float mShift = 0;
};

struct MADGINE_TREES_EXPORT FormatTreeBase {

    void format(float distance = 1);

    enum class Order {
        PreOrder,
        PostOrder
    };

    void visit(const std::function<void(void *, float, float)> &visitor, Order order = Order::PreOrder);

private:
    static void move_subtree(FormatNode &wl, FormatNode &wr, float shift);

    static FormatNode *apportion(FormatNode &dt, FormatNode *defaultAncestor, float distance);

    static void first_walk(FormatNode &dt, float distance = 1);

    static void second_walk(FormatNode &dt, float m = 0, int depth = 0);

protected:
    FormatNode mRoot = nullptr;
};

template <typename T>
struct FormatTree : FormatTreeBase {

    template <typename F>
    FormatTree(T &t, F &&childrenAccessor)
    {
        filler(t, mRoot, std::forward<F>(childrenAccessor));
    }

    template <typename F>
    void visit(F &&f)
    {
        FormatTreeBase::visit([&](void *id, float x, float y) {
            f(static_cast<T *>(id), x, y);
        });
    }

private:
    template <typename F>
    void filler(T &t, FormatNode &node, F &&childrenAccessor)
    {
        for (T &child : std::invoke(childrenAccessor, t)) {
            filler(child, node.addChild(&child), std::forward<F>(childrenAccessor));
        }
    }
};

}