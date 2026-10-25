#include "../../widgetslib.h"

#include "formattingstack.h"

#include "Madgine/render/texture.h"

#include "widgetsrenderdata.h"

namespace Engine {
namespace Widgets {

    FormattingStack::FormattingStack(const Render::TypeFace *typeFace, Render::FontStyle style, float fontSize, ColorFrame color)
        : mTypeFace(typeFace)
        , mStyle(style)
        , mFontSize(fontSize)
        , mColor(color)
    {
    }

    FormattingStack::~FormattingStack()
    {
        //assert(!mFormatting);
        //assert(!mEscaped);
    }

    float FormattingStack::renderGlyph(WidgetsRenderData &renderData, char32_t code, const Math::Vector2 &pos)
    {
        const Render::Glyph &glyph = mTypeFace->mFonts.at(mStyle)[code > 0xFE ? '?' : static_cast<unsigned char>(code)];

        bool useSmallSize = mFontSize < 32;

        TextureSettings tex { mTypeFace->mTexture->resourceBlock(), useSmallSize ? 0 : TextureFlag_IsDistanceField };

        float scale = mFontSize / Render::FontLoader::sFontSize;
        float width = glyph.mSize.x * scale;
        float height = glyph.mSize.y * scale;

        float startX = glyph.mBearing.x * scale;
        float startY = -glyph.mBearing.y * scale;

        if (useSmallSize)
            renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { width, height }, mColor, tex, { glyph.mUV2, glyph.mSize2 }, mTypeFace->mTexture->size(), glyph.mFlipped2);
        else
            renderData.renderQuadUV({ pos.x + startX, pos.y + startY }, { width, height }, mColor, tex, { glyph.mUV, glyph.mSize }, mTypeFace->mTexture->size(), glyph.mFlipped);

        return glyph.mAdvance / 64.0f * scale;
    }

    void FormattingStack::renderShadow(WidgetsRenderData &renderData, char32_t code, const Math::Vector2 &pos, const Math::Vector2 &offset)        
    {
        float scale = mFontSize / Render::FontLoader::sFontSize;
        ColorFrame shadowFrame = ColorRenderData { Math::Color4 { 0.0f, 0.0f, 0.0f, mColor.mRenderData.mColor.a } }.frame(mColor.mPos, mColor.mSize);
        std::swap(mColor, shadowFrame);
        renderGlyph(renderData, code, pos + offset * scale);
        std::swap(mColor, shadowFrame);
    }

    float FormattingStack::advance(char32_t code)
    {
        const Render::Glyph &g = mTypeFace->mFonts.at(mStyle)[code > 0xFE ? '?' : static_cast<unsigned char>(code)];

        float scale = mFontSize / Render::FontLoader::sFontSize;

        return g.mAdvance / 64.0f * scale;
    }

    std::pair<float, float> FormattingStack::place(float height)
    {
        float scale = mFontSize / Render::FontLoader::sFontSize;
        float minY = mTypeFace->mDescender / 64.0f * scale;
        float maxY = mTypeFace->mAscender / 64.0f * scale;
        float fullHeight = maxY - minY;

        return { height <= maxY + minY ? (maxY + minY + height) * 0.5f : (std::min(height, fullHeight) + (maxY + minY)) * 0.5f, std::min(height, fullHeight) / height };
    }

    std::pair<bool, std::string> FormattingStack::parse(char32_t token)
    {
        bool consumed = false;
        std::string image;

        if (mEscaped) {
            //assert(token == '\\' || token == '<');
            mEscaped = false;
        } else if (mFormatting) {
            //assert(token != '<');
            if (token == '>') {
                mFormatting = false;
                auto sep = mFormatToken.find('=');
                if (sep != std::string_view::npos) {
                    std::string_view command { mFormatToken };
                    std::string_view type = StringUtil::trim(command.substr(0, sep));
                    std::string_view arg = StringUtil::trim(command.substr(sep + 1));
                    if (type == "color") {
                        uint32_t intColor;
                        auto result = std::from_chars(arg.data() + 1, arg.data() + arg.size() - 1, intColor, 16);

                        Math::Color4 targetColor { 0.0f, 0.0f, 0.0f, 1.0f };
                        if (arg.size() >= 10) {
                            targetColor.a = (intColor & 0xFF) / 255.0f;
                            intColor >>= 8;
                        }
                        targetColor.b = (intColor & 0xFF) / 255.0f;
                        intColor >>= 8;
                        targetColor.g = (intColor & 0xFF) / 255.0f;
                        intColor >>= 8;
                        targetColor.r = (intColor & 0xFF) / 255.0f;

                        mColorStack.push({ mColor.mRenderData.mColor, mColor.mRenderData.mSecondaryColor });

                        mColor.mRenderData.mColor = targetColor;
                        mColor.mRenderData.mSecondaryColor = targetColor;

                    } else if (type == "size") {
                        mSizeStack.push(mFontSize);
                        auto result = std::from_chars(arg.data(), arg.data() + arg.size(), mFontSize);
                    } else if (type == "icon") {
                        image = std::string { arg.substr(1, arg.size() - 2) };
                    } else {
                        // Failed
                    }
                } else if (mFormatToken == "bold") {
                    mStyle = Render::FontStyle::Bold;
                } else if (mFormatToken == "/bold") {
                    mStyle = Render::FontStyle::Default;
                } else if (mFormatToken == "/color") {
                    if (!mColorStack.empty()) {
                        mColor.mRenderData.mColor = mColorStack.top().first;
                        mColor.mRenderData.mSecondaryColor = mColorStack.top().second;
                        mColorStack.pop();
                    }
                } else if (mFormatToken == "/size") {
                    if (!mSizeStack.empty()) {
                        mFontSize = mSizeStack.top();
                        mSizeStack.pop();
                    }
                } else {
                }
            } else {
                mFormatToken += token;
            }
            consumed = true;
        } else {
            if (token == '\\') {
                mEscaped = true;
                consumed = true;
            } else if (token == '<') {
                mFormatting = true;
                mFormatToken.clear();
                consumed = true;
            }
        }

        return { consumed, std::move(image) };
    }

    const Render::TypeFace *FormattingStack::typeFace() const
    {
        return mTypeFace;
    }

}
}