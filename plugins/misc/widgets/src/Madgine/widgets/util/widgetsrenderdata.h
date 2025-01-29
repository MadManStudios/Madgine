#pragma once

#include "texturesettings.h"
#include "vertex.h"

#include "Meta/math/color4.h"

#include "Meta/math/rect2i.h"
#include "Meta/math/rect2.h"

namespace Engine {
namespace Widgets {

    struct WidgetsVertexData {
        std::vector<Vertex> mTriangleVertices;

        void renderQuad(Vector3 pos, Vector2 size, const Rect2 &clipRect, Color4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, Vector2 topLeftUV = { 0.0f, 0.0f }, Vector2 bottomRightUV = { 1.0f, 1.0f }, bool flippedUV = false);
        void renderQuadUV(Vector3 pos, Vector2 size, const Rect2 &clipRect, Color4 color, Rect2i rect, Vector2i textureSize, bool flippedUV = false);
    };

    struct WidgetsLinesData {
        std::vector<Vertex> mLineVertices;

        void renderLine(Line3 line, const Rect2 &clipRect, Color4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
    };

    struct WidgetsRenderData {
        

        void renderQuad(Vector3 pos, Vector2 size, Color4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, TextureSettings tex = {}, Vector2 topLeftUV = { 0.0f, 0.0f }, Vector2 bottomRightUV = { 1.0f, 1.0f }, bool flippedUV = false);
        void renderQuadUV(Vector3 pos, Vector2 size, Color4 color, TextureSettings tex, Rect2i rect, Vector2i textureSize, bool flippedUV = false);

        void renderLine(const Line3 &line, Color4 color = { 1.0f, 1.0f, 1.0f, 1.0f });

        const std::map<TextureSettings, WidgetsVertexData> &vertexData() const;
        const std::vector<Vertex> &lineVertices() const;

        struct WidgetsRenderDataClipRectKeep {
            ~WidgetsRenderDataClipRectKeep();
            Rect2 mOldRect;
            WidgetsRenderData &mRenderData;
        };
        WidgetsRenderDataClipRectKeep pushClipRect(Vector2 pos, Vector2 size);

    private:
        std::map<TextureSettings, WidgetsVertexData> mVertexData;
        WidgetsLinesData mLineData;
        Rect2 mClipRect {
            { 0.0f, 0.0f },
            { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() }
        };
    };

}
}