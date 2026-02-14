#include "../../renderlib.h"

#include "fontloader.h"

#include "Generic/areaview.h"
#include "Generic/bytebuffer.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Meta/math/atlas2.h"
#include "Meta/math/vector2i.h"
#include "Meta/math/vector3.h"
#include "Meta/serialize/container/container_operations.h"
#include "Meta/serialize/formats.h"
#include "Meta/serialize/operations.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Madgine/render/rendercontext.h"
#include "Madgine/serialize/filesystem/filemanager.h"
#include "Madgine/serialize/memory/memorymanager.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#undef INFINITE
#include "core/edge-coloring.h"
#include "msdfgen.h"

#ifdef STATIC_BUILD
#    undef DLL_EXPORT
#    undef DLL_IMPORT
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#ifdef STATIC_BUILD
#    define DLL_EXPORT
#    define DLL_IMPORT
#endif

RESOURCELOADER(Engine::Render::FontLoader)

SERIALIZETABLE_BEGIN(Engine::Rect2i)
    FIELD(mTopLeft)
    FIELD(mSize)
SERIALIZETABLE_END(Engine::Rect2i)

SERIALIZETABLE_BEGIN(Engine::Atlas2::Entry)
    FIELD(mArea)
    FIELD(mFlipped)
SERIALIZETABLE_END(Engine::Atlas2::Entry)

namespace Engine {
namespace Render {

    namespace msdfgen {

        struct FtContext {
            ::msdfgen::Point2 position;
            ::msdfgen::Shape *shape;
            ::msdfgen::Contour *contour;
        };

        static ::msdfgen::Point2 ftPoint2(const FT_Vector &vector)
        {
            return ::msdfgen::Point2(vector.x / 64., vector.y / 64.);
        }

        static int ftMoveTo(const FT_Vector *to, void *user)
        {
            FtContext *context = reinterpret_cast<FtContext *>(user);
            context->contour = &context->shape->addContour();
            context->position = ftPoint2(*to);
            return 0;
        }

        static int ftLineTo(const FT_Vector *to, void *user)
        {
            FtContext *context = reinterpret_cast<FtContext *>(user);
            context->contour->addEdge(new ::msdfgen::LinearSegment(context->position, ftPoint2(*to)));
            context->position = ftPoint2(*to);
            return 0;
        }

        static int ftConicTo(const FT_Vector *control, const FT_Vector *to, void *user)
        {
            FtContext *context = reinterpret_cast<FtContext *>(user);
            context->contour->addEdge(new ::msdfgen::QuadraticSegment(context->position, ftPoint2(*control), ftPoint2(*to)));
            context->position = ftPoint2(*to);
            return 0;
        }

        static int ftCubicTo(const FT_Vector *control1, const FT_Vector *control2, const FT_Vector *to, void *user)
        {
            FtContext *context = reinterpret_cast<FtContext *>(user);
            context->contour->addEdge(new ::msdfgen::CubicSegment(context->position, ftPoint2(*control1), ftPoint2(*control2), ftPoint2(*to)));
            context->position = ftPoint2(*to);
            return 0;
        }
    }

    static bool ignore(unsigned char c, Render::Glyph *g = nullptr)
    {
        if (c == '\r') {
            if (g)
                g->mAdvance = 0;
            return true;
        } else if (c == '\t') {
            if (g)
                g->mAdvance = 128 * FontLoader::sFontSize;
            return true;
        } else {
            return false;
        }
    }

    FontLoader::FontLoader()
        : ResourceLoader({ ".msdf", ".ttf" }, { .mIconName = "FontIcon.png" })
    {
    }

    Threading::Task<bool> FontLoader::init()
    {
        if (!co_await ResourceLoader::init())
            co_return false;

        if (FT_Init_FreeType(&mFreeType)) {
            LOG_ERROR("FREETYPE: Could not init FreeType Library");
            co_return false;
        }

        co_return true;
    }

    Threading::Task<void> FontLoader::finalize()
    {
        FT_Done_FreeType(mFreeType);

        co_await ResourceLoader::finalize();
    }

    Threading::Task<bool> FontLoader::loadImpl(TypeFace &typeFace, ResourceDataInfo &info)
    {

        if (info.resource()->path().extension() == ".msdf") {

            auto fileBufferResult = co_await info.resource()->readAsync();
            std::stringstream errorReason;
            if (fileBufferResult.is_value()) {
                ByteBuffer fileBuffer = std::move(fileBufferResult).value();

                Memory::MemoryManager cache("msdf_cache");
                Serialize::FormattedSerializeStream in = cache.openRead(std::move(fileBuffer), Serialize::Formats::safebinary);
                assert(in);
                ByteBuffer b;
                Vector2i textureSize;
                Serialize::StreamResult result = [&]() {
                    STREAM_PROPAGATE_ERROR(read(in, typeFace.mFonts, nullptr));
                    STREAM_PROPAGATE_ERROR(read(in, textureSize, nullptr));
                    STREAM_PROPAGATE_ERROR(read(in, typeFace.mAscender, nullptr));
                    STREAM_PROPAGATE_ERROR(read(in, typeFace.mDescender, nullptr));
                    return read(in, b, nullptr);
                }();
                if (result.mState == Serialize::StreamState::OK) {
                    typeFace.mTexture = RenderContext::getSingleton().createTexture(TextureType_2D, FORMAT_RGBA8, textureSize, std::move(b));
                    co_return true;
                }
                errorReason << result;
            } else if (fileBufferResult.is_error()) {
                errorReason << std::move(fileBufferResult).error().mError;
            } else {
                errorReason << "Cancelled";
            }
            LOG_ERROR("Failed to load \"" << info.resource()->path() << "\": \n"
                                          << errorReason.str());
            LOG("Falling back to .ttf file load");
        }
        Filesystem::Path path = info.resource()->path();
        if (path.extension() == ".msdf") {
            path = path.parentPath() / (std::string { path.stem() } + ".ttf");
        }
        assert(path.extension() == ".ttf");

        LOG("Creating Cache for " << path);

        constexpr int UNIT_SIZE = 256;

        Atlas2 atlas({ UNIT_SIZE, UNIT_SIZE });
        atlas.addBin({ 0, 0 });

        int areaSize = 1;

        auto expand = [&]() {
            for (int i = 0; i < areaSize; ++i) {
                for (int j = 0; j < areaSize; ++j) {
                    atlas.addBin({ j * UNIT_SIZE, (areaSize + i) * UNIT_SIZE });
                    atlas.addBin({ (areaSize + j) * UNIT_SIZE, i * UNIT_SIZE });
                    atlas.addBin({ (areaSize + j) * UNIT_SIZE, (areaSize + i) * UNIT_SIZE });
                }
            }
            areaSize *= 2;
        };

        struct TempData {
            std::array<Vector2i, TypeFace::sFontGlyphCount> mSizes;
            std::array<Vector2i, TypeFace::sFontGlyphCount> mLowResSizes;
            std::vector<Atlas2::Entry> mEntries;
            std::vector<Atlas2::Entry> mLowResEntries;
            FT_Face mFace;
            ByteBuffer mBuffer;
        };

        std::map<FontStyle, TempData> tempData;
        for (FontStyle style : { FontStyle::Default, FontStyle::Italic, FontStyle::Bold, FontStyle::Light }) {

            static constexpr auto sStyleNames = std::array { "Bold", "Italic", "Light" };
            std::string styleName = "";
            for (size_t i = 0; i < sStyleNames.size(); i++) {
                if (style & (1 << i)) {
                    styleName += sStyleNames[i];
                    break;
                }
            }
            if (styleName.empty())
                styleName = "Regular";

            Filesystem::Path stylePath = path.parentPath() / (std::string { path.stem() } + "-" + styleName + std::string { path.extension() });

            if (!Filesystem::exists(stylePath)) {
                if (style == FontStyle::Default) {
                    if (Filesystem::exists(path)) {
                        stylePath = path;
                    } else {
                        LOG_ERROR("FREETYPE: Regular Font file does not exist: " << path);
                        co_return false;
                    }
                } else {
                    continue;
                }
            }

            TempData &data = tempData[style];
            data.mBuffer = (co_await Filesystem::readFileAsync(stylePath)).value();

            if (FT_New_Memory_Face(mFreeType, static_cast<const FT_Byte *>(data.mBuffer.mData), data.mBuffer.mSize, 0, &data.mFace)) {
                LOG_ERROR("FREETYPE: Failed to load font");
                co_return false;
            }

            FT_Set_Pixel_Sizes(data.mFace, 0, sFontSize);

            // TODO
            typeFace.mAscender = data.mFace->size->metrics.ascender;
            typeFace.mDescender = data.mFace->size->metrics.descender;

            std::array<Vector2i, TypeFace::sFontGlyphCount> extendedSizes;

            for (unsigned char c = 0; c < TypeFace::sFontGlyphCount; c++) {
                if (ignore(c))
                    continue;
                // Load character glyph
                if (FT_Load_Char(data.mFace, c, FT_LOAD_DEFAULT)) {
                    LOG_ERROR("FREETYTPE: Failed to load Glyph");
                    data.mSizes[c] = { 0, 0 };
                    extendedSizes[c] = { 0, 0 };
                    continue;
                }
                data.mSizes[c] = { static_cast<int>(data.mFace->glyph->bitmap.width) + 4, static_cast<int>(data.mFace->glyph->bitmap.rows) + 4 };
                extendedSizes[c] = { static_cast<int>(data.mFace->glyph->bitmap.width) + 5, static_cast<int>(data.mFace->glyph->bitmap.rows) + 5 };
            }

            data.mEntries = atlas.insert(
                extendedSizes, expand, true);

            FT_Set_Pixel_Sizes(data.mFace, 0, 24);

            for (unsigned char c = 0; c < TypeFace::sFontGlyphCount; c++) {
                if (ignore(c))
                    continue;

                // Load character glyph
                if (FT_Load_Char(data.mFace, c, FT_LOAD_DEFAULT)) {
                    LOG_ERROR("FREETYTPE: Failed to load Glyph");
                    data.mSizes[c] = { 0, 0 };
                    extendedSizes[c] = { 0, 0 };
                    continue;
                }
                data.mLowResSizes[c] = { static_cast<int>(data.mFace->glyph->bitmap.width), static_cast<int>(data.mFace->glyph->bitmap.rows) };
                extendedSizes[c] = { static_cast<int>(data.mFace->glyph->bitmap.width) + 1, static_cast<int>(data.mFace->glyph->bitmap.rows) + 1 };
            }

            data.mLowResEntries = atlas.insert(
                extendedSizes, expand, true);
        }

        Vector2i textureSize = { areaSize * UNIT_SIZE,
            areaSize * UNIT_SIZE };
        size_t byteSize = textureSize.x * textureSize.y;
        std::unique_ptr<std::array<unsigned char, 4>[]> texBuffer = std::make_unique<std::array<unsigned char, 4>[]>(byteSize);
        AreaView<std::array<unsigned char, 4>, 2> tex { texBuffer.get(), { static_cast<size_t>(textureSize.x), static_cast<size_t>(textureSize.y) } };

        for (const auto &[style, data] : tempData) {

            TypeFace::Font &font = typeFace.mFonts[style];

            FT_Set_Pixel_Sizes(data.mFace, 0, sFontSize);

            for (unsigned char c = 0; c < TypeFace::sFontGlyphCount; c++) {
                if (ignore(c, &font[c]))
                    continue;

                // Load character glyph
                if (FT_Load_Char(data.mFace, c, FT_LOAD_RENDER)) {
                    LOG_ERROR("FREETYTPE: Failed to load Glyph");
                    continue;
                }

                std::unique_ptr<Vector3[]> buffer = std::make_unique<Vector3[]>(data.mSizes[c].x * data.mSizes[c].y);
                AreaView<Vector3, 2> bufferView { buffer.get(), { static_cast<size_t>(data.mSizes[c].x), static_cast<size_t>(data.mSizes[c].y) } };

                ::msdfgen::BitmapRef<float, 3>
                    bm { buffer[0].ptr(), data.mSizes[c].x, data.mSizes[c].y };

                ::msdfgen::Shape shape;
                shape.inverseYAxis = true;

                msdfgen::FtContext context = {};
                context.shape = &shape;
                FT_Outline_Funcs ftFunctions;
                ftFunctions.move_to = &msdfgen::ftMoveTo;
                ftFunctions.line_to = &msdfgen::ftLineTo;
                ftFunctions.conic_to = &msdfgen::ftConicTo;
                ftFunctions.cubic_to = &msdfgen::ftCubicTo;
                ftFunctions.shift = 0;
                ftFunctions.delta = 0;
                FT_Outline_Decompose(&data.mFace->glyph->outline, &ftFunctions, &context);

                ::msdfgen::edgeColoringSimple(shape, 3);
                ::msdfgen::generateMSDF(bm, shape, 4.0, { 1, 1 }, { static_cast<double>(-data.mFace->glyph->bitmap_left + 2), static_cast<double>(data.mSizes[c].y - data.mFace->glyph->bitmap_top - 2) });

                font[c].mSize = data.mSizes[c];
                font[c].mUV = data.mEntries[c].mArea.mTopLeft;
                font[c].mFlipped = data.mEntries[c].mFlipped;
                font[c].mAdvance = data.mFace->glyph->advance.x;
                font[c].mBearing.x = data.mFace->glyph->bitmap_left - 1;
                font[c].mBearing.y = data.mFace->glyph->bitmap_top - 1;

                Vector2i size = data.mSizes[c];
                if (data.mEntries[c].mFlipped)
                    std::swap(size.x, size.y);
                Vector2i pos = { data.mEntries[c].mArea.mTopLeft.x, data.mEntries[c].mArea.mTopLeft.y };

                AreaView<std::array<unsigned char, 4>, 2> targetView = tex.subArea({ static_cast<size_t>(pos.x), static_cast<size_t>(pos.y) }, { static_cast<size_t>(size.x), static_cast<size_t>(size.y) });
                if (data.mEntries[c].mFlipped)
                    targetView.swapAxis(0, 1);

                std::ranges::transform(bufferView, targetView.begin(),
                    [](const Vector3 &v) {
                        return std::array<unsigned char, 4> {
                            static_cast<unsigned char>(clamp(v.x, 0.0f, 1.0f) * 255),
                            static_cast<unsigned char>(clamp(v.y, 0.0f, 1.0f) * 255),
                            static_cast<unsigned char>(clamp(v.z, 0.0f, 1.0f) * 255),
                            255
                        };
                    });
            }

            FT_Set_Pixel_Sizes(data.mFace, 0, 24);

            for (unsigned char c = 0; c < TypeFace::sFontGlyphCount; c++) {
                if (ignore(c))
                    continue;

                // Load character glyph
                if (FT_Load_Char(data.mFace, c, FT_LOAD_RENDER)) {
                    LOG_ERROR("FREETYTPE: Failed to load Glyph");
                    continue;
                }

                if (FT_Render_Glyph(data.mFace->glyph, FT_RENDER_MODE_NORMAL)) {
                    LOG_ERROR("FREETYPE: Failed to render Glyph");
                    continue;
                }

                AreaView<unsigned char, 2> bufferView { data.mFace->glyph->bitmap.buffer, { static_cast<size_t>(data.mLowResSizes[c].x), static_cast<size_t>(data.mLowResSizes[c].y) } };

                font[c].mSize2 = data.mLowResSizes[c] + Vector2i(2, 2);
                font[c].mUV2 = data.mLowResEntries[c].mArea.mTopLeft - Vector2i(1, 1);
                font[c].mFlipped2 = data.mLowResEntries[c].mFlipped;

                Vector2i size = data.mLowResSizes[c];
                if (data.mLowResEntries[c].mFlipped)
                    std::swap(size.x, size.y);
                Vector2i pos = { data.mLowResEntries[c].mArea.mTopLeft.x, data.mLowResEntries[c].mArea.mTopLeft.y };

                AreaView<std::array<unsigned char, 4>, 2> targetView = tex.subArea({ static_cast<size_t>(pos.x), static_cast<size_t>(pos.y) }, { static_cast<size_t>(size.x), static_cast<size_t>(size.y) });
                if (data.mLowResEntries[c].mFlipped)
                    targetView.swapAxis(0, 1);

                std::ranges::transform(bufferView, targetView.begin(),
                    [](const unsigned char f) {
                        return std::array<unsigned char, 4> {
                            255,
                            255,
                            255,
                            f
                        };
                    });
            }

            FT_Done_Face(data.mFace);
        }

        typeFace.mTexture = RenderContext::getSingleton().createTexture(TextureType_2D, FORMAT_RGBA8, textureSize, { texBuffer.get(), 4 * byteSize });

        Filesystem::FileManager cache("msdf_cache");
        Serialize::FormattedSerializeStream out = cache.openWrite(info.resource()->path().parentPath() / (std::string { info.resource()->name() } + ".msdf"), Serialize::Formats::safebinary);
        if (out) {
            write(out, typeFace.mFonts, "fonts");
            write(out, textureSize, "size");
            write(out, typeFace.mAscender, "ascender");
            write(out, typeFace.mDescender, "descender");
            write(out, ByteBuffer { texBuffer.get(), 4 * byteSize }, "texture");
        }

        co_return true;
    }

    void FontLoader::unloadImpl(TypeFace &typeFace)
    {
        typeFace.mTexture.reset();
    }

    Threading::TaskQueue *FontLoader::loadingTaskQueue() const
    {
        return RenderContext::renderQueue();
    }

    std::pair<Resources::ResourceBase *, bool> FontLoader::addResource(const Filesystem::Path &path, std::string_view name)
    {
        if (path.extension() == ".msdf") {
            return ResourceLoader::addResource(path, name);
        } else if (path.extension() == ".ttf") {
            std::string_view typefaceName = path.stem();

            auto it = typefaceName.find('-');
            if (it == std::string_view::npos) {
                LOG_WARNING("Font file \"" << path << "\" does not follow the naming convention \"<typeface>-<style>.ttf\". This may cause issues with caching and loading.");
            } else {
                std::string_view style = typefaceName.substr(it + 1);
                typefaceName = typefaceName.substr(0, it);

                if (style != "Regular" && style != "Italic" && style != "Bold") {
                    LOG_WARNING("Font file \"" << path << "\" has an unrecognized style \"" << style << "\". This may cause issues with caching and loading.");
                }
            }

            return ResourceLoader::addResource(path.parentPath() / (std::string { typefaceName } + ".ttf"), typefaceName);
        } else {
            throw 0;
        }
    }

}
}