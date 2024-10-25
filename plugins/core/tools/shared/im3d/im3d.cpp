#include "Meta/metalib.h"
#include "Modules/moduleslib.h"

#include "im3d.h"

#include "Madgine/render/vertex.h"

#include "im3d_internal.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Meta/math/geometry3.h"

#include "Meta/math/boundingbox.h"

#include "Meta/math/quaternion.h"

#include "Madgine/fontloader/glyph.h"

#include "Meta/math/transformation.h"

#include <numeric>

#include "Modules/threading/workgroupstorage.h"

namespace Engine {

namespace Im3D {

    Threading::WorkgroupLocal<Im3DContext *> sContext;

    void CreateContext()
    {
        assert(sContext == nullptr);
        sContext = new Im3DContext;
    }

    void DestroyContext()
    {
        assert(sContext != nullptr);
        delete sContext;
        sContext = nullptr;
    }

    Im3DContext *GetCurrentContext()
    {
        return sContext;
    }

    Im3DIO &GetIO()
    {
        return sContext->mIO;
    }

    const Ray3 &GetMouseRay()
    {
        return sContext->mMouseRay;
    }

    Im3DObject::Im3DObject(Im3DID id)
        : mID(id)
    {
    }

    void NewFrame()
    {
        Im3DContext &c = *sContext;

        for (std::pair<const Im3DTextureId, Im3DContext::RenderData> &p : c.mRenderData) {
            for (size_t i = 0; i < IM3D_MESHTYPE_COUNT; ++i) {
                p.second.mIndices[i].clear();
                p.second.mVertices[i].clear();
                p.second.mVertexBase[i] = 0;
                for (auto it = p.second.mPersistentMeshes[i].begin(); it != p.second.mPersistentMeshes[i].end();) {
                    if (std::chrono::steady_clock::now() <= std::get<0>(*it)) {
                        std::ranges::transform(std::get<2>(*it), std::back_inserter(p.second.mIndices[i]), [&](unsigned short j) { return j + p.second.mVertices[i].size(); });
                        std::ranges::copy(std::get<1>(*it), std::back_inserter(p.second.mVertices[i]));
                        ++it;
                    } else {
                        it = p.second.mPersistentMeshes[i].erase(it);
                    }
                }
                p.second.mIndices2[i].clear();
                p.second.mVertices2[i].clear();
                p.second.mVertexBase2[i] = 0;
            }
        }

        for (std::pair<const Im3DNativeMesh, std::vector<Matrix4>> &p : c.mNativeMeshes)
            p.second.clear();

        assert(c.mIDStack.empty());

        c.mHoveredObject = c.mNextHoveredObject;
        c.mNextHoveredObject = nullptr;
        c.mNextHoveredPriority = 0;
        c.mNextHoveredDistance = std::numeric_limits<float>::max();

        Im3DIO &io = c.mIO;
        c.mMouseRay = io.mNextFrameMouseRay;
        io.mNextFrameMouseRay = {};

        c.mTemp.mLastObject = nullptr;
        c.mTemp.mLastAABB = {};
        c.mTemp.mLastTransform = Matrix4::IDENTITY;
    }

    Im3DObject *FindObjectByID(Im3DID id)
    {
        Im3DContext &c = *sContext;
        auto it = c.mObjects.find(id);
        if (it == c.mObjects.end())
            return nullptr;
        return &it->second;
    }

    Im3DObject *FindObjectByName(const char *name)
    {
        ImGuiID id = ImHashStr(name);
        return FindObjectByID(id);
    }

    Im3DID GetID(const char *name)
    {
        Im3DContext &c = *sContext;
        Im3DID seed = c.mIDStack.empty() ? 0 : c.mIDStack.back();
        Im3DID id = ImHashStr(name, 0, seed);
        return id;
    }

    Im3DID GetID(const void *ptr)
    {
        Im3DContext &c = *sContext;
        ImGuiID seed = c.mIDStack.empty() ? 0 : c.mIDStack.back();
        return ImHashData(&ptr, sizeof(void *), seed);
    }

    void PushID(const void *ptr)
    {
        Im3DContext &c = *sContext;
        c.mIDStack.push_back(GetID(ptr));
    }

    void PopID()
    {
        Im3DContext &c = *sContext;
        c.mIDStack.pop_back();
    }

    Im3DObject *CreateObject(Im3DID id)
    {
        Im3DContext &c = *sContext;

        Im3DObject *o = &c.mObjects.try_emplace(id, id).first->second;

        return o;
    }

    static void ResetLastObject()
    {
        Im3DContext &c = *sContext;

        for (std::pair<const Im3DTextureId, Im3DContext::RenderData> &p : c.mRenderData) {
            for (size_t i = 0; i < IM3D_MESHTYPE_COUNT; ++i) {
                p.second.mVertexBase[i] = p.second.mVertices[i].size();
            }
            for (size_t i = 0; i < IM3D_MESHTYPE_COUNT; ++i) {
                p.second.mVertexBase2[i] = p.second.mVertices2[i].size();
            }
        }
    }

    void Mesh(Im3DMeshType type, const Vertex *vertices, size_t vertexCount, const MeshParameters &param, const uint32_t *indices, size_t indexCount)
    {
        Im3DContext &c = *sContext;

        ResetLastObject();

        Vector3 minP { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        Vector3 maxP { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

        size_t vertexPivot = c.mRenderData[0].mVertexBase[type];
        size_t indexPivot = c.mRenderData[0].mIndices[type].size();

        std::transform(vertices, vertices + vertexCount, std::back_inserter(c.mRenderData[0].mVertices[type]), [&](const Vertex &v) {
            Vertex result = v;

            minP = min(v.mPos, minP);
            maxP = max(v.mPos, maxP);

            result.mPos = (param.mTransform * Vector4 { v.mPos, 1.0f }).xyz();

            /*if (c.mHoveredObject == object)
                result.mColor *= 2.f;*/

            return result;
        });

        c.mTemp.mLastAABB = { minP, maxP };
        c.mTemp.mLastTransform = param.mTransform;

        size_t groupSize = (size_t)type + 1;

        if (indices) {
            assert(indexCount % groupSize == 0);
            std::transform(indices, indices + indexCount, std::back_inserter(c.mRenderData[0].mIndices[type]), [&](uint32_t i) { return i + vertexPivot; });
        } else {
            assert(vertexCount % groupSize == 0);
            c.mRenderData[0].mIndices[type].resize(indexPivot + vertexCount);
            std::iota(c.mRenderData[0].mIndices[type].begin() + indexPivot, c.mRenderData[0].mIndices[type].end(), vertexPivot);
        }

        if (param.mDuration != std::chrono::microseconds::zero()) {
            auto &data = c.mRenderData[0].mPersistentMeshes[type].emplace_back(std::chrono::steady_clock::now() + param.mDuration, std::vector<Vertex> { c.mRenderData[0].mVertices[type].begin() + vertexPivot, c.mRenderData[0].mVertices[type].end() }, std::vector<unsigned short> {});
            std::transform(c.mRenderData[0].mIndices[type].begin() + indexPivot, c.mRenderData[0].mIndices[type].end(), std::back_inserter(std::get<2>(data)), [&](unsigned int i) { return i - vertexPivot; });
        }

        assert(c.mRenderData[0].mIndices[type].size() % groupSize == 0);
    }

    void Mesh(Im3DMeshType type, Render::RenderPassFlags flags, const Vertex2 *vertices, size_t vertexCount, const MeshParameters &param, const uint32_t *indices, size_t indexCount, Im3DTextureId texId)
    {
        Im3DContext &c = *sContext;

        ResetLastObject();

        Vector3 minP { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
        Vector3 maxP { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };

        std::transform(vertices, vertices + vertexCount, std::back_inserter(c.mRenderData[texId].mVertices2[type]), [&](const Vertex2 &v) {
            Vertex2 result = v;

            minP = min(v.mPos, minP);
            maxP = max(v.mPos, maxP);

            result.mPos = (param.mTransform * Vector4 { v.mPos, 1.0f }).xyz();

            /*if (c.mHoveredObject == object)
                result.mColor *= 2.f;*/

            return result;
        });

        c.mTemp.mLastAABB = { minP, maxP };
        c.mTemp.mLastTransform = param.mTransform;

        size_t groupSize = (size_t)type + 1;

        if (indices) {
            assert(indexCount % groupSize == 0);
            std::transform(indices, indices + indexCount, std::back_inserter(c.mRenderData[texId].mIndices2[type]), [&](uint32_t i) { return i + c.mRenderData[texId].mVertexBase2[type]; });
        } else {
            assert(vertexCount % groupSize == 0);
            size_t oldIndexCount = c.mRenderData[texId].mIndices2[type].size();
            c.mRenderData[texId].mIndices2[type].resize(oldIndexCount + vertexCount);
            std::iota(c.mRenderData[texId].mIndices2[type].begin() + oldIndexCount, c.mRenderData[texId].mIndices2[type].end(), c.mRenderData[texId].mVertexBase2[type]);
        }
        c.mRenderData[texId].mFlags = flags;

        assert(c.mRenderData[texId].mIndices2[type].size() % groupSize == 0);
    }

    void NativeMesh(Im3DNativeMesh mesh, const AABB &bb, const Matrix4 &transform)
    {
        Im3DContext &c = *sContext;

        ResetLastObject();

        c.mNativeMeshes[mesh].push_back(transform);

        c.mTemp.mLastAABB = bb;
        c.mTemp.mLastTransform = transform;
    }

    void Text(const char *text, const TextParameters &param)
    {
        size_t textLen = strlen(text);

        if (textLen == 0)
            return;

        Im3DFont font = GetIO().mFetchFont(param.mFontName);

        if (!font.mTexture)
            return;

        float scale = param.mFontSize / 5000.0f;

        float fullWidth = 0.0f;
        float minY = 0.0f;
        float maxY = 0.0f;

        for (size_t i = 0; i < textLen; ++i) {
            Render::Glyph &g = font.mGlyphs[static_cast<uint8_t>(text[i])];

            fullWidth += g.mAdvance / 64.0f * scale;
            maxY = max(maxY, g.mBearing.y * scale);
            minY = min(minY, (g.mBearing.y - g.mSize.y) * scale);
        }

        float fullHeight = maxY - minY;

        float cursorX = -fullWidth * param.mPivot.x;
        float originY = fullHeight * param.mPivot.y - maxY;

        std::unique_ptr<Vertex2[]> vertices = std::make_unique<Vertex2[]>(4 * textLen);
        std::unique_ptr<uint32_t[]> indices = std::make_unique<uint32_t[]>(6 * textLen);

        for (size_t i = 0; i < textLen; ++i) {
            Render::Glyph &g = font.mGlyphs[static_cast<uint8_t>(text[i])];

            float width = g.mSize.x * scale;
            float height = g.mSize.y * scale;

            float vPosX1 = cursorX + g.mBearing.x * scale;
            float vPosX2 = vPosX1 + width;
            float vPosY1 = originY + g.mBearing.y * scale;
            float vPosY2 = vPosY1 - height;

            Vector3 v11 = { 0, 0, 0 }, v12 = { 0, 0, 0 }, v21 = { 0, 0, 0 }, v22 = { 0, 0, 0 };
            Vector2 v211 = { 0, 0 }, v212 = { 0, 0 }, v221 = { 0, 0 }, v222 = { 0, 0 };

            if (param.mFacingX) {
                v211.x = vPosX1;
                v212.x = vPosX2;
                v221.x = vPosX1;
                v222.x = vPosX2;
            } else {
                v11.x = vPosX1;
                v12.x = vPosX2;
                v21.x = vPosX1;
                v22.x = vPosX2;
            }

            if (param.mFacingY) {
                v211.y = vPosY1;
                v212.y = vPosY1;
                v221.y = vPosY2;
                v222.y = vPosY2;
            } else {
                v11.y = vPosY1;
                v12.y = vPosY1;
                v21.y = vPosY2;
                v22.y = vPosY2;
            }

            int uvWidth = g.mSize.x;
            int uvHeight = g.mSize.y;

            if (g.mFlipped)
                std::swap(uvWidth, uvHeight);

            Vector2 uvTopLeft = { float(g.mUV.x) / font.mTextureSize.x, float(g.mUV.y) / font.mTextureSize.y };
            Vector2 uvBottomRight = { float(g.mUV.x + uvWidth) / font.mTextureSize.x,
                float(g.mUV.y + uvHeight) / font.mTextureSize.y };

            Vector2 uvTopRight = { uvBottomRight.x, uvTopLeft.y };
            Vector2 uvBottomLeft = { uvTopLeft.x, uvBottomRight.y };

            if (g.mFlipped)
                std::swap(uvTopRight, uvBottomLeft);

            vertices[4 * i] = { v11, v211, { 1, 1, 1, 1 }, uvTopLeft };
            vertices[4 * i + 1] = { v12, v212, { 1, 1, 1, 1 }, uvTopRight };
            vertices[4 * i + 2] = { v21, v221, { 1, 1, 1, 1 }, uvBottomLeft };
            vertices[4 * i + 3] = { v22, v222, { 1, 1, 1, 1 }, uvBottomRight };

            indices[6 * i] = 4 * i;
            indices[6 * i + 1] = 4 * i + 1;
            indices[6 * i + 2] = 4 * i + 2;
            indices[6 * i + 3] = 4 * i + 2;
            indices[6 * i + 4] = 4 * i + 1;
            indices[6 * i + 5] = 4 * i + 3;

            cursorX += g.mAdvance / 64.0f * scale;
        }

        Mesh(IM3D_TRIANGLES, Render::RenderPassFlags_NoLighting | Render::RenderPassFlags_DistanceField, vertices.get(), 4 * textLen, param, indices.get(), 6 * textLen, font.mTexture);

        if (GetIO().mReleaseFont)
            GetIO().mReleaseFont(font);
    }

    void Line(const Vector3 &a, const Vector3 &b, const LineParameters &param)
    {
        Vertex vertices[] = {
            { a, param.mColorA },
            { b, param.mColorB }
        };
        Mesh(IM3D_LINES, vertices, 2, param);
    }

    void Arrow(float radius, const Vector3 &a, const Vector3 &b, const Parameters &param)
    {
        constexpr int segments = 6;

        Vertex vertices[2 + segments];
        uint32_t indices[2 + 2 * segments];

        Vector3 d = b - a;

        vertices[0] = {
            a, param.mColor
        };
        indices[0] = 0;
        vertices[1] = {
            b, param.mColor
        };
        indices[1] = 1;

        Vector3 p = d.perpendicular();
        p.normalize();
        p -= d.normalizedCopy();
        p *= radius /* / sqrt2*/;

        for (int i = 0; i < segments; ++i) {
            Vector3 dir = Quaternion { i * 2 * PI / segments, d } * p;
            vertices[2 + i] = { b + dir, param.mColor };
            indices[2 + 2 * i] = 1;
            indices[2 + 2 * i + 1] = 2 + i;
        }

        Mesh(IM3D_LINES, vertices, 2 + segments, param, indices, 2 + 2 * segments);
    }

    void Arrow3D(Im3DMeshType type, float radius, const Vector3 &a, const Vector3 &b, const Parameters &param)
    {
        Vector3 dist = b - a;
        Vector3 d1 = dist.perpendicular();
        d1.normalize();
        d1 *= radius;
        Vector3 d2 = d1.crossProduct(dist);
        d2.normalize();
        d2 *= radius;
        Vector3 d0 = radius * dist.normalizedCopy();

        const Vertex vertices[]
            = { { a, param.mColor },
                  { a + d0 + d1, param.mColor },
                  { a + d0 + d2, param.mColor },
                  { a + d0 - d1, param.mColor },
                  { a + d0 - d2, param.mColor },
                  { b, param.mColor } };

        switch (type) {
        case IM3D_TRIANGLES: {
            constexpr uint32_t indices[] = {
                0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1, 1, 2, 5, 2, 3, 5, 3, 4, 5, 4, 1, 5
            };

            Mesh(IM3D_TRIANGLES, vertices, 6, param, indices, 24);
            break;
        }
        case IM3D_LINES: {
            constexpr uint32_t indices[] = {
                0, 1, 0, 2, 0, 3, 0, 4, 1, 5, 2, 5, 3, 5, 4, 5, 1, 2, 2, 3, 3, 4, 4, 1
            };

            Mesh(IM3D_LINES, vertices, 6, param, indices, 24);
            break;
        }
        default:
            std::terminate();
        }
    }

    void Sphere(const Vector3 &center, float radius, const SphereParameters &param)
    {
        size_t faceVertexCount = (param.mDetail + 2) * (param.mDetail + 3) / 2 - 3;
        size_t vertexCount = 20 * faceVertexCount + 12;
        std::unique_ptr<Vertex[]> vertices = std::make_unique<Vertex[]>(vertexCount);

        size_t indexCount = 60 * (param.mDetail + 1) * (param.mDetail + 2);
        std::unique_ptr<uint32_t[]> indices = std::make_unique<uint32_t[]>(indexCount);

        auto vertexIndex = [=](size_t face, size_t x, size_t y) {
            return 12 + face * faceVertexCount + ((5 * y + 2 * param.mDetail * y - y * y) / 2) - (y > 0 ? 2 : 1) + x;
        };

        constexpr float coeff = (1.0f + sqrtf(5.0f)) / 2.0f;
        constexpr float coeff2 = sqrtf(2.5f + sqrtf(5.0f) / 2.0f);
        radius /= coeff2;
        const float t = radius * coeff;

        vertices[0] = {
            Vector3 { -radius, t, 0 },
            param.mColor
        };
        vertices[1] = {
            Vector3 { radius, t, 0 },            
            param.mColor
        };
        vertices[2] = {
            Vector3 { -radius, -t, 0 },            
            param.mColor
        };
        vertices[3] = {
            Vector3 { radius, -t, 0 },
            param.mColor
        };
        vertices[4] = {
            Vector3 { 0, -radius, t },            
            param.mColor
        };
        vertices[5] = {
            Vector3 { 0, radius, t },            
            param.mColor
        };
        vertices[6] = {
            Vector3 { 0, -radius, -t },            
            param.mColor
        };
        vertices[7] = {
            Vector3 { 0, radius, -t },            
            param.mColor
        };
        vertices[8] = {
            Vector3 { t, 0, -radius },            
            param.mColor
        };
        vertices[9] = {
            Vector3 { t, 0, radius },            
            param.mColor
        };
        vertices[10] = {
            Vector3 { -t, 0, -radius },            
            param.mColor
        };
        vertices[11] = {
            Vector3 { -t, 0, radius },            
            param.mColor
        };

        constexpr uint32_t cornerIndices[20][3] = {
            { 0, 11, 5 },
            { 0, 5, 1 },
            { 0, 1, 7 },
            { 0, 7, 10 },
            { 0, 10, 11 },
            { 1, 5, 9 },
            { 5, 11, 4 },
            { 11, 10, 2 },
            { 10, 7, 6 },
            { 7, 1, 8 },
            { 3, 9, 4 },
            { 3, 4, 2 },
            { 3, 2, 6 },
            { 3, 6, 8 },
            { 3, 8, 9 },
            { 4, 9, 5 },
            { 2, 4, 11 },
            { 6, 2, 10 },
            { 8, 6, 7 },
            { 9, 8, 1 }
        };

        size_t vertexCounter = 12;
        for (size_t face = 0; face < 20; ++face) {
            Vector3 bottomLeft = vertices[cornerIndices[face][0]].mPos;
            Vector3 bottomRight = vertices[cornerIndices[face][1]].mPos;
            Vector3 top = vertices[cornerIndices[face][2]].mPos;
            for (size_t y = 0; y <= param.mDetail + 1; ++y) {

                Vector3 left = slerp(bottomLeft, top, y / float(param.mDetail + 1));
                Vector3 right = slerp(bottomRight, top, y / float(param.mDetail + 1));

                for (size_t x = 0; x <= param.mDetail + 1 - y; ++x) {
                    if ((y == 0 && (x == 0 || x == param.mDetail + 1)) || y == param.mDetail + 1)
                        continue;
                    vertices[vertexCounter++] = {
                        slerp(left, right, x / float(param.mDetail + 1 - y)),                        
                        param.mColor
                    };
                }
            }
        }
        assert(vertexCounter == vertexCount);

        size_t indexCounter = 0;
        for (size_t face = 0; face < 20; ++face) {
            for (size_t y = 0; y <= param.mDetail; ++y) {
                for (size_t x = 0; x <= param.mDetail - y; ++x) {
                    unsigned short bottomLeft = (x == 0 && y == 0) ? cornerIndices[face][0] : vertexIndex(face, x, y);
                    unsigned short bottomRight = (x == param.mDetail && y == 0) ? cornerIndices[face][1] : vertexIndex(face, x + 1, y);
                    unsigned short top = (y == param.mDetail) ? cornerIndices[face][2] : vertexIndex(face, x, y + 1);
                    indices[indexCounter++] = bottomLeft;
                    indices[indexCounter++] = bottomRight;
                    indices[indexCounter++] = bottomLeft;
                    indices[indexCounter++] = top;
                    indices[indexCounter++] = top;
                    indices[indexCounter++] = bottomRight;
                }
            }
        }
        assert(indexCounter == indexCount);

        MeshParameters meshParam = param;
        meshParam.mTransform = meshParam.mTransform * TranslationMatrix(center);
        Mesh(IM3D_LINES, vertices.get(), vertexCount, meshParam, indices.get(), indexCounter);
    }

    MADGINE_IM3D_EXPORT void Frustum(const struct Frustum &frustum, const Parameters &param)
    {
        Vertex vertices[8];
        auto corners = frustum.getCorners();
        for (size_t i = 0; i < 8; ++i) {
            vertices[i].mPos = corners[i];
            vertices[i].mColor = param.mColor;
        }

        static constexpr uint32_t indices[] = {
            0, 1, 0, 3, 0, 4, 1, 2, 1, 5, 2, 3, 2, 6, 3, 7, 4, 5, 4, 7, 5, 6, 6, 7
        };

        Mesh(IM3D_LINES, vertices, 8, param, indices, 24);
    }

    bool BoundingSphere(const char *name, Im3DBoundingObjectFlags flags, size_t priority)
    {
        return BoundingSphere(GetID(name), flags, priority);
    }

    bool BoundingSphere(Im3DID id, Im3DBoundingObjectFlags flags, size_t priority)
    {
        Im3DContext &c = *sContext;
        return BoundingSphere(id, c.mTemp.mLastAABB, c.mTemp.mLastTransform, flags, priority);
    }

    bool BoundingSphere(const char *name, const AABB &bb, const Matrix4 &transform, Im3DBoundingObjectFlags flags, size_t priority)
    {
        return BoundingSphere(GetID(name), bb, transform, flags, priority);
    }

    bool BoundingSphere(Im3DID id, const AABB &bb, const Matrix4 &transform, Im3DBoundingObjectFlags flags, size_t priority)
    {
        Im3DContext &c = *sContext;

        Engine::Sphere bounds = { bb.center(),
            0.4f * bb.diameter() };

        //Check if Hovered
        float distance = 0.0f;
        if (auto intersection = Intersect(c.mMouseRay, transform * bounds))
            distance = intersection[0];

        return BoundingObject(id, distance, flags, priority);
    }

    bool BoundingBox(const char *name, Im3DBoundingObjectFlags flags, size_t priority)
    {
        return BoundingBox(GetID(name), flags, priority);
    }

    bool BoundingBox(Im3DID id, Im3DBoundingObjectFlags flags, size_t priority)
    {
        Im3DContext &c = *sContext;
        return BoundingBox(id, c.mTemp.mLastAABB, c.mTemp.mLastTransform, flags, priority);
    }

    bool BoundingBox(const char *name, const AABB &bb, const Matrix4 &transform, Im3DBoundingObjectFlags flags, size_t priority)
    {
        return BoundingBox(GetID(name), bb, transform, flags, priority);
    }

    bool BoundingBox(Im3DID id, const AABB &bb, const Matrix4 &transform, Im3DBoundingObjectFlags flags, size_t priority)
    {
        Im3DContext &c = *sContext;

        //Check if Hovered
        float distance = 0.0f;
        if (auto intersection = Intersect(c.mMouseRay, transform * bb))
            distance = intersection[0];

        bool hovered = BoundingObject(id, distance, flags, priority);

        if ((flags & Im3DBoundingObjectFlags_ShowOutline) || (hovered && (flags & Im3DBoundingObjectFlags_ShowOnHover))) {
            std::array<Vector3, 8> corners = bb.corners();
            Vertex vertices[] = {
                { corners[0], { 1, 1, 1, 1 } },
                { corners[1], { 1, 1, 1, 1 } },
                { corners[2], { 1, 1, 1, 1 } },
                { corners[3], { 1, 1, 1, 1 } },
                { corners[4], { 1, 1, 1, 1 } },
                { corners[5], { 1, 1, 1, 1 } },
                { corners[6], { 1, 1, 1, 1 } },
                { corners[7], { 1, 1, 1, 1 } },
            };
            constexpr uint32_t indices[] = {
                0, 1, 0, 2, 0, 4, 1, 3, 1, 5, 2, 3, 2, 6, 3, 7, 4, 5, 4, 6, 5, 7, 6, 7
            };
            Mesh(IM3D_LINES, vertices, 8, { .mTransform = transform }, indices, 24);
        }

        return hovered;
    }

    bool BoundingObject(Im3DID id, float distance, Im3DBoundingObjectFlags flags, size_t priority)
    {
        Im3DContext &c = *sContext;

        Im3DObject *object = CreateObject(id);

        if (distance > 0.0f && ((priority == c.mNextHoveredPriority && distance < c.mNextHoveredDistance) || priority > c.mNextHoveredPriority)) {
            c.mNextHoveredObject = object;
            c.mNextHoveredDistance = distance;
            c.mNextHoveredPriority = priority;
        }

        if (c.mHoveredObject == object) {
            for (std::pair<const Im3DTextureId, Im3DContext::RenderData> &p : c.mRenderData) {
                for (size_t t = 0; t < IM3D_MESHTYPE_COUNT; ++t) {
                    for (size_t i = p.second.mVertexBase[t]; i < p.second.mVertices[t].size(); ++i) {
                        p.second.mVertices[t][i].mColor *= 2.f;
                    }
                }
            }
        }

        c.mTemp.mLastObject = object;

        return object == c.mHoveredObject;
    }

    bool IsObjectHovered()
    {
        Im3DContext &c = *sContext;
        return c.mTemp.mLastObject && c.mTemp.mLastObject == c.mHoveredObject;
    }

    bool IsAnyObjectHovered()
    {
        Im3DContext &c = *sContext;
        return c.mHoveredObject || c.mNextHoveredObject;
    }

}
}