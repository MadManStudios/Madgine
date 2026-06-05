#pragma once

#include "Meta/math/color4.h"
#include "Meta/math/rect2.h"
#include "Meta/math/rect2i.h"

#include "colorrenderdata.h"
#include "texturesettings.h"
#include "vertex.h"

namespace Engine {
namespace Widgets {

    struct WidgetsVertexData {
        std::vector<Vertex> mTriangleVertices;

        void renderQuad(Math::Vector3 pos, Math::Vector2 size, const Math::Rect2 &clipRect, ColorFrame color = {}, Math::Vector2 topLeftUV = { 0.0f, 0.0f }, Math::Vector2 bottomRightUV = { 1.0f, 1.0f }, bool flippedUV = false);
        void renderQuadUV(Math::Vector3 pos, Math::Vector2 size, const Math::Rect2 &clipRect, ColorFrame color, Math::Rect2i rect, Math::Vector2i textureSize, bool flippedUV = false);
    };

    struct WidgetsLinesData {
        std::vector<Vertex> mLineVertices;

        void renderLine(Math::Line3 line, const Math::Rect2 &clipRect, Math::Color4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
    };

    struct MADGINE_WIDGETS_EXPORT WidgetsRenderData {

        void renderQuad(Math::Vector2 pos, Math::Vector2 size, ColorFrame color = {}, TextureSettings tex = {}, Math::Vector2 topLeftUV = { 0.0f, 0.0f }, Math::Vector2 bottomRightUV = { 1.0f, 1.0f }, bool flippedUV = false, bool transparentContent = false);
        void renderQuadUV(Math::Vector2 pos, Math::Vector2 size, ColorFrame color, TextureSettings tex, Math::Rect2i rect, Math::Vector2i textureSize, bool flippedUV = false, bool transparentContent = false);

        void renderLine(const Math::Line2 &line, Math::Color4 color = { 1.0f, 1.0f, 1.0f, 1.0f });

        const std::map<size_t, std::map<TextureSettings, WidgetsVertexData>> &vertexData() const;
        const std::vector<Vertex> &lineVertices() const;

        struct WidgetsRenderDataClipRectKeep {
            ~WidgetsRenderDataClipRectKeep();
            Math::Rect2 mOldRect;
            WidgetsRenderData &mRenderData;
        };
        WidgetsRenderDataClipRectKeep pushClipRect(Math::Vector2 pos, Math::Vector2 size);

        void setAlpha(float alpha);
        float alpha() const;

        void setLayer(size_t layer);
        size_t layer() const;

        void setSubLayer(size_t layer);
        size_t subLayer() const;

    protected:
        WidgetsVertexData &vertexData(const TextureSettings &tex, bool transparentContent);

    private:
        std::map<size_t, std::map<TextureSettings, WidgetsVertexData>> mVertexData;
        WidgetsLinesData mLineData;
        Math::Rect2 mClipRect {
            { 0.0f, 0.0f },
            { std::numeric_limits<float>::max(), std::numeric_limits<float>::max() }
        };
        float mAlpha = 1.0f;
        size_t mLayer = 0;
        size_t mSubLayer = 0;
    };

}
}