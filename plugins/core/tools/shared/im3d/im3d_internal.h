#pragma once

#include "Meta/math/boundingbox.h"
#include "Meta/math/sphere.h"

#include "Madgine/render/renderpassflags.h"

namespace Engine {
namespace Im3D {

    constexpr size_t MAX_PRIORITY_COUNT = 3;

    struct Im3DObject {
        Im3DObject(Im3DID id);

        Im3DID mID;
    };

    struct Im3DObjectTempData {
        Im3DObject *mLastObject;
        AABB mLastAABB;
        Matrix4 mLastTransform;
    };

    struct Im3DContext {

        Im3DIO mIO;

        struct RenderData {
            std::vector<Vertex> mVertices[IM3D_MESHTYPE_COUNT];
            std::vector<uint32_t> mIndices[IM3D_MESHTYPE_COUNT];
            size_t mVertexBase[IM3D_MESHTYPE_COUNT];
            std::list<std::tuple<std::chrono::steady_clock::time_point, std::vector<Vertex>, std::vector<unsigned short>>> mPersistentMeshes[IM3D_MESHTYPE_COUNT];
            std::vector<Vertex2> mVertices2[IM3D_MESHTYPE_COUNT];
            std::vector<uint32_t> mIndices2[IM3D_MESHTYPE_COUNT];
            size_t mVertexBase2[IM3D_MESHTYPE_COUNT];
            Render::RenderPassFlags mFlags = Render::RenderPassFlags_NoLighting;
        };
        std::map<Im3DTextureId, RenderData> mRenderData;

        std::map<Im3DNativeMesh, std::vector<Matrix4>> mNativeMeshes;

        std::map<Im3DID, Im3DObject> mObjects;
        std::vector<Im3DID> mIDStack;

        Ray3 mMouseRay;
        Im3DObject *mHoveredObject;
        Im3DObject *mNextHoveredObject;
        float mNextHoveredDistance;
        size_t mNextHoveredPriority;

        Im3DObjectTempData mTemp;
    };

    MADGINE_IM3D_EXPORT Im3DObject *FindObjectByID(Im3DID id);

    MADGINE_IM3D_EXPORT Im3DObject *FindObjectByName(const char *name);

    MADGINE_IM3D_EXPORT Im3DID GetID(const char *name);
    MADGINE_IM3D_EXPORT Im3DID GetID(const void *ptr);
    
}
}