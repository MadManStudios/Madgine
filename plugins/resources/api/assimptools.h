#pragma once

#include "Meta/math/matrix4.h"

#include <assimp/matrix4x4.h>

namespace Engine {
namespace Render {

    inline Matrix4 assimpConvertMatrix(const aiMatrix4x4 &m)
    {
        return {
            m.a1, m.a2, m.a3, m.a4,
            m.b1, m.b2, m.b3, m.b4,
            m.c1, m.c2, m.c3, m.c4,
            m.d1, m.d2, m.d3, m.d4
        };
    }

    inline Matrix4 assimpCalculateTransformMatrix(aiNode *node)
    {
        Matrix4 m = Matrix4::IDENTITY;
        while (node) {
            m = m * assimpConvertMatrix(node->mTransformation);
            node = node->mParent;
        }
        return m;
    }
		
    template <typename F>
    void assimpTraverseTree(const aiNode *node, const F &f, Matrix4 m)
    {
        m = assimpConvertMatrix(node->mTransformation) * m;
        TupleUnpacker::invoke(f, node, m);
        for (size_t i = 0; i < node->mNumChildren; ++i) {
            assimpTraverseTree(node->mChildren[i], f, m);
        }
    }

    template <typename F>
    void assimpTraverseTree(const aiScene *scene, const F &f, const Matrix4 &m = Matrix4::IDENTITY)
    {
        assimpTraverseTree(scene->mRootNode, f, m);
    }

}
}