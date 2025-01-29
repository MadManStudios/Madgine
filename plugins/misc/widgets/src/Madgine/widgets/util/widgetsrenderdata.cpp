#include "../../widgetslib.h"

#include "widgetsrenderdata.h"

#include "Meta/math/geometry2.h"
#include "Meta/math/line3.h"

namespace Engine {
namespace Widgets {

    void WidgetsVertexData::renderQuad(Vector3 pos, Vector2 size, const Rect2 &clipRect, Color4 color, Vector2 topLeftUV, Vector2 bottomRightUV, bool flippedUV)
    {
        if (flippedUV) {
            std::swap(topLeftUV.x, topLeftUV.y);
            std::swap(bottomRightUV.x, bottomRightUV.y);
        }

        //clipping
        // boundary checks
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

        Vector2 topRightUV = { bottomRightUV.x, topLeftUV.y };
        Vector2 bottomLeftUV = { topLeftUV.x, bottomRightUV.y };

        if (flippedUV)
            std::swap(topRightUV, bottomLeftUV);

        Vector3 v = pos;
        mTriangleVertices.push_back({ v, color, topLeftUV });
        v.x += size.x;
        mTriangleVertices.push_back({ v, color, topRightUV });
        v.y += size.y;
        mTriangleVertices.push_back({ v, color, bottomRightUV });
        mTriangleVertices.push_back({ v, color, bottomRightUV });
        v.x -= size.x;
        mTriangleVertices.push_back({ v, color, bottomLeftUV });
        v.y -= size.y;
        mTriangleVertices.push_back({ v, color, topLeftUV });
    }

    void WidgetsVertexData::renderQuadUV(Vector3 pos, Vector2 size, const Rect2 &clipRect, Color4 color, Rect2i rect, Vector2i textureSize, bool flippedUV)
    {
        if (flippedUV)
            std::swap(rect.mSize.x, rect.mSize.y);

        Vector2 topLeftUV = Vector2 { rect.mTopLeft + Vector2i { 1, 1 } } / Vector2 { textureSize };
        Vector2 bottomRightUV = Vector2 { rect.mTopLeft + rect.mSize - Vector2i { 1, 1 } } / Vector2 { textureSize };

        renderQuad(pos, size, clipRect, color, topLeftUV, bottomRightUV, flippedUV);
    }

    void WidgetsLinesData::renderLine(Line3 line, const Rect2 &clipRect, Color4 color)
    {
        Line2 line2 = line.xy();
        bool aIsInClipRect = clipRect.contains(line2.mPointA, true);
        if (auto result = Intersect(line2, clipRect)) {
            Vector2 lineStart = line2.point(result[0]);
            Vector2 lineEnd = result.size() > 1 ? line2.point(result[1]) : (aIsInClipRect ? line2.mPointA : line2.mPointB);
            line = { Vector3 { lineStart, line.mPointA.z }, Vector3 { lineEnd, line.mPointB.z } };
        } else {
            if (!aIsInClipRect)
                return;
        }

        mLineVertices.push_back({ line.mPointA, color, { 0, 0 } });
        mLineVertices.push_back({ line.mPointB, color, { 0, 0 } });
    }

    void WidgetsRenderData::renderQuad(Vector3 pos, Vector2 size, Color4 color, TextureSettings tex, Vector2 topLeftUV, Vector2 bottomRightUV, bool flippedUV)
    {
        mVertexData[tex].renderQuad(pos, size, mClipRect, color, topLeftUV, bottomRightUV, flippedUV);
    }

    void WidgetsRenderData::renderQuadUV(Vector3 pos, Vector2 size, Color4 color, TextureSettings tex, Rect2i rect, Vector2i textureSize, bool flippedUV)
    {
        mVertexData[tex].renderQuadUV(pos, size, mClipRect, color, rect, textureSize, flippedUV);
    }

    void WidgetsRenderData::renderLine(const Line3 &line, Color4 color)
    {
        mLineData.renderLine(line, mClipRect, color);
    }

    const std::map<TextureSettings, WidgetsVertexData> &WidgetsRenderData::vertexData() const
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

    WidgetsRenderData::WidgetsRenderDataClipRectKeep WidgetsRenderData::pushClipRect(Vector2 pos, Vector2 size)
    {
        WidgetsRenderDataClipRectKeep keep {
            mClipRect,
            *this
        };
        mClipRect = { pos, size };
        return keep;
    }

}
}