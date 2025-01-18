#include "treeslib.h"

#include "treeformat.h"

namespace Engine {

FormatNode::FormatNode(void *id, FormatNode *parent, size_t number)
    : mId(id)
    , mParent(parent)
    , mNumber(number)
{
}

FormatNode &FormatNode::addChild(void *id)
{
    return mChildren.emplace_back(id, this, mChildren.size() + 1);
}

void FormatNode::visitPreOrder(const std::function<void(void *, float, float)> &visitor)
{
    visitor(mId, mX, mY);
    for (FormatNode &node : mChildren) {
        node.visitPreOrder(visitor);
    }
}

void FormatNode::visitPostOrder(const std::function<void(void *, float, float)> &visitor)
{
    for (FormatNode &node : mChildren) {
        node.visitPostOrder(visitor);
    }
    visitor(mId, mX, mY);
}

FormatNode *FormatNode::left_brother()
{
    FormatNode *result = nullptr;
    if (mParent) {
        for (FormatNode &tree : mParent->mChildren) {
            if (&tree == this)
                return result;
            else
                result = &tree;
        }
    }
    return result;
}

FormatNode *FormatNode::get_lmost_sibling()
{
    if (!mLMostSibling && mParent && this != &mParent->mChildren.front())
        mLMostSibling = &mParent->mChildren.front();
    return mLMostSibling;
}

FormatNode *FormatNode::left()
{
    if (mThread)
        return mThread;
    else
        return mChildren.empty() ? nullptr : &mChildren.front();
}

FormatNode *FormatNode::right()
{
    if (mThread)
        return mThread;
    else
        return mChildren.empty() ? nullptr : &mChildren.back();
}

FormatNode *FormatNode::ancestor(FormatNode &v, FormatNode *defaultAncestor)
{
    auto it = std::find_if(v.mParent->mChildren.begin(), v.mParent->mChildren.end(), [this](FormatNode &w) { return &w == mAncestor; });
    if (it != v.mParent->mChildren.end())
        return mAncestor;
    else
        return defaultAncestor;
}

void FormatNode::execute_shifts()
{
    float shift = 0;
    float change = 0;
    for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
        FormatNode &w = *it;
        w.mX += shift;
        w.mMod += shift;
        change += w.mChange;
        shift += w.mShift + change;
        w.mChange = 0;
        w.mShift = 0;
    }
}

void FormatTreeBase::format(float distance)
{
    first_walk(mRoot, distance);
    second_walk(mRoot);
}

void FormatTreeBase::visit(const std::function<void(void *, float, float)> &visitor, Order order)
{
    switch (order) {
    case Order::PreOrder:
        mRoot.visitPreOrder(visitor);
        break;
    case Order::PostOrder:
        mRoot.visitPostOrder(visitor);
        break;
    }
}

void FormatTreeBase::move_subtree(FormatNode &wl, FormatNode &wr, float shift)
{
    size_t subtrees = wr.mNumber - wl.mNumber;
    // std::cout << "Tree (" << wl.y << ", " << wl.mNumber << ") is conflicted with tree (" << wr.y << ", " << wr.mNumber << "), moving " << subtrees << ", shift " << shift << std::endl;
    wr.mChange -= shift / subtrees;
    wr.mShift += shift;
    wl.mChange += shift / subtrees;
    wr.mX += shift;
    wr.mMod += shift;
}

FormatNode *FormatTreeBase::apportion(FormatNode &dt, FormatNode *defaultAncestor, float distance)
{
    FormatNode *w = dt.left_brother();
    if (w) {
        FormatNode *vir = &dt;
        FormatNode *vor = &dt;
        FormatNode *vil = w;
        FormatNode *vol = dt.get_lmost_sibling();
        float sir = dt.mMod;
        float sor = dt.mMod;
        float sil = vil->mMod;
        float sol = vol->mMod;
        while (vil->right() && vir->left()) {
            vil = vil->right();
            vir = vir->left();
            vol = vol->left();
            vor = vor->right();
            vor->mAncestor = &dt;
            float shift = (vil->mX + sil) - (vir->mX + sir) + distance;
            if (shift > 0) {
                move_subtree(*vil->ancestor(dt, defaultAncestor), dt, shift);
                sir = sir + shift;
                sor = sor + shift;
            }
            sil += vil->mMod;
            sir += vir->mMod;
            sol += vol->mMod;
            sor += vor->mMod;
        }
        if (vil->right() && !vor->right()) {
            vor->mThread = vil->right();
            vor->mMod += sil - sor;
        } else {
            if (vir->left() && !vol->left()) {
                vol->mThread = vir->left();
                vol->mMod += sir - sol;
            }
            defaultAncestor = &dt;
        }
    }
    return defaultAncestor;
}

void FormatTreeBase::first_walk(FormatNode &dt, float distance)
{
    if (dt.mChildren.empty()) {
        if (dt.get_lmost_sibling())
            dt.mX = dt.left_brother()->mX + distance;
        else
            dt.mX = 0;
    } else {
        FormatNode *defaultAncestor = &dt.mChildren.front();
        for (FormatNode &w : dt.mChildren) {
            first_walk(w, distance);
            defaultAncestor = apportion(w, defaultAncestor, distance);
        }

        dt.execute_shifts();

        float midpoint = (dt.mChildren.front().mX + dt.mChildren.back().mX) / 2.0f;

        FormatNode *w = dt.left_brother();
        if (w) {
            dt.mX = w->mX + distance;
            dt.mMod = dt.mX - midpoint;
        } else {
            dt.mX = midpoint;
        }
    }
}

void FormatTreeBase::second_walk(FormatNode &dt, float m, int depth)
{
    dt.mX += m;
    dt.mY = depth;

    for (FormatNode &w : dt.mChildren)
        second_walk(w, m + dt.mMod, depth + 1);

    dt.mMod = 0;
    dt.mThread = nullptr;
}

}