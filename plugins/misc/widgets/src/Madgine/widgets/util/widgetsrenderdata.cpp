#include "../../widgetslib.h"

#include "widgetsrenderdata.h"

#include "Meta/math/geometry2.h"
#include "Meta/math/line3.h"

namespace Engine {
namespace Widgets {

    void WidgetsVertexData::renderQuad(Math::Vector3 pos, Math::Vector2 size, const Math::Rect2 &clipRect, ColorFrame color, Math::Vector2 topLeftUV, Math::Vector2 bottomRightUV, bool flippedUV)
    {
        if (flippedUV) {
            std::swap(topLeftUV.x, topLeftUV.y);
            std::swap(bottomRightUV.x, bottomRightUV.y);
        }

        // clipping
        //  boundary checks
        if (
            pos.x + size.x < clipRect.mTopLeft.x
            || pos.y + size.y < clipRect.mTopLeft.y
            || pos.x > clipRect.right()
            || pos.y > clipRect.bottom()) {
            return;
        }

        if (pos.x < clipRect.mTopLeft.x) {
            float diff = clipRect.mTopLeft.x - pos.x;
            float ratio = diff / size.x;
            topLeftUV.x += ratio * (bottomRightUV.x - topLeftUV.x);
            size.x -= diff;
            pos.x = clipRect.mTopLeft.x;
        }
        if (pos.x + size.x > clipRect.right()) {
            float diff = pos.x + size.x - clipRect.right();
            float ratio = diff / size.x;
            bottomRightUV.x -= ratio * (bottomRightUV.x - topLeftUV.x);
            size.x -= diff;
        }
        if (pos.y < clipRect.mTopLeft.y) {
            float diff = clipRect.mTopLeft.y - pos.y;
            float ratio = diff / size.y;
            topLeftUV.y += ratio * (bottomRightUV.y - topLeftUV.y);
            size.y -= diff;
            pos.y = clipRect.mTopLeft.y;
        }
        if (pos.y + size.y > clipRect.bottom()) {
            float diff = pos.y + size.y - clipRect.bottom();
            float ratio = diff / size.y;
            bottomRightUV.y -= ratio * (bottomRightUV.y - topLeftUV.y);
            size.y -= diff;
        }

        if (flippedUV) {
            std::swap(topLeftUV.x, topLeftUV.y);
            std::swap(bottomRightUV.x, bottomRightUV.y);
        }

        Math::Vector2 topRightUV = { bottomRightUV.x, topLeftUV.y };
        Math::Vector2 bottomLeftUV = { topLeftUV.x, bottomRightUV.y };

        if (flippedUV)
            std::swap(topRightUV, bottomLeftUV);

        ColorQuad quad = color.toQuad(pos.xy(), size);

        Math::Vector3 v = pos;
        mTriangleVertices.push_back({ v, quad.mTopLeft, topLeftUV });
        v.x += size.x;
        mTriangleVertices.push_back({ v, quad.mTopRight, topRightUV });
        v.y += size.y;
        mTriangleVertices.push_back({ v, quad.mBottomRight, bottomRightUV });
        mTriangleVertices.push_back({ v, quad.mBottomRight, bottomRightUV });
        v.x -= size.x;
        mTriangleVertices.push_back({ v, quad.mBottomLeft, bottomLeftUV });
        v.y -= size.y;
        mTriangleVertices.push_back({ v, quad.mTopLeft, topLeftUV });
    }

    void WidgetsVertexData::renderQuadUV(Math::Vector3 pos, Math::Vector2 size, const Math::Rect2 &clipRect, ColorFrame color, Math::Rect2i rect, Math::Vector2i textureSize, bool flippedUV)
    {
        if (flippedUV)
            std::swap(rect.mSize.x, rect.mSize.y);

        Math::Vector2 topLeftUV = Math::Vector2 { rect.mTopLeft + Math::Vector2i { 1, 1 } } / Math::Vector2 { textureSize };
        Math::Vector2 bottomRightUV = Math::Vector2 { rect.mTopLeft + rect.mSize - Math::Vector2i { 1, 1 } } / Math::Vector2 { textureSize };

        renderQuad(pos, size, clipRect, color, topLeftUV, bottomRightUV, flippedUV);
    }

    void WidgetsLinesData::renderLine(Math::Line3 line, const Math::Rect2 &clipRect, Math::Color4 color)
    {
        Math::Line2 line2 = line.xy();
        bool aIsInClipRect = clipRect.contains(line2.mPointA, true);
        if (auto result = Intersect(line2, clipRect)) {
            Math::Vector2 lineStart = line2.point(result[0]);
            Math::Vector2 lineEnd = result.size() > 1 ? line2.point(result[1]) : (aIsInClipRect ? line2.mPointA : line2.mPointB);
            line = { Math::Vector3 { lineStart, line.mPointA.z }, Math::Vector3 { lineEnd, line.mPointB.z } };
        } else {
            if (!aIsInClipRect)
                return;
        }

        mLineVertices.push_back({ line.mPointA, color, { 0, 0 } });
        mLineVertices.push_back({ line.mPointB, color, { 0, 0 } });
    }

    void WidgetsRenderData::renderQuad(Math::Vector2 pos, Math::Vector2 size, ColorFrame color, TextureSettings tex, Math::Vector2 topLeftUV, Math::Vector2 bottomRightUV, bool flippedUV, bool transparentContent)
    {
        color.mRenderData.mColor.a *= mAlpha;
        vertexData(tex, transparentContent || color.mRenderData.mColor.a < 1.0f).renderQuad(Math::Vector3 { pos, 10.0f * mLayer + mSubLayer }, size, mClipRect, color, topLeftUV, bottomRightUV, flippedUV);
    }

    void WidgetsRenderData::renderQuadUV(Math::Vector2 pos, Math::Vector2 size, ColorFrame color, TextureSettings tex, Math::Rect2i rect, Math::Vector2i textureSize, bool flippedUV, bool transparentContent)
    {
        color.mRenderData.mColor.a *= mAlpha;
        vertexData(tex, transparentContent || color.mRenderData.mColor.a < 1.0f).renderQuadUV(Math::Vector3 { pos, 10.0f * mLayer + mSubLayer }, size, mClipRect, color, rect, textureSize, flippedUV);
    }

    void WidgetsRenderData::renderLine(const Math::Line2 &line, Math::Color4 color)
    {
        color.a *= mAlpha;
        mLineData.renderLine({ Math::Vector3 { line.mPointA, 10.0f * mLayer + mSubLayer }, Math::Vector3 { line.mPointB, 10.0f * mLayer + mSubLayer } }, mClipRect, color);
    }

    const std::map<size_t, std::map<TextureSettings, WidgetsVertexData>> &WidgetsRenderData::vertexData() const
    {
        return mVertexData;
    }

    const std::vector<Vertex> &WidgetsRenderData::lineVertices() const
    {
        return mLineData.mLineVertices;
    }

    WidgetsRenderData::WidgetsRenderDataClipRectKeep::~WidgetsRenderDataClipRectKeep()
    {
        mRenderData.mClipRect = mOldRect;
    }

    WidgetsRenderData::WidgetsRenderDataClipRectKeep WidgetsRenderData::pushClipRect(Math::Vector2 pos, Math::Vector2 size)
    {
        WidgetsRenderDataClipRectKeep keep {
            mClipRect,
            *this
        };
        mClipRect = { pos, size };
        return keep;
    }

    WidgetsVertexData &WidgetsRenderData::vertexData(const TextureSettings &tex, bool transparentContent)
    {
        return mVertexData[transparentContent ? 10 * mLayer + mSubLayer : 0][tex];
    }

    void WidgetsRenderData::setAlpha(float alpha)
    {
        mAlpha = alpha;
    }

    float WidgetsRenderData::alpha() const
    {
        return mAlpha;
    }

    void WidgetsRenderData::setLayer(size_t layer)
    {
        mLayer = layer;
        mSubLayer = 0;
    }

    size_t WidgetsRenderData::layer() const
    {
        return mLayer;
    }

    void WidgetsRenderData::setSubLayer(size_t layer)
    {
        mSubLayer = layer;
    }

    size_t WidgetsRenderData::subLayer() const
    {
        return mSubLayer;
    }

}
}