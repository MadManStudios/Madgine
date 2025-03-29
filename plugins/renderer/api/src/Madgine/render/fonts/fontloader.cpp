#include "../../renderlib.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "fontloader.h"

#include "Meta/math/atlas2.h"
#include "Meta/math/vector2i.h"

#include "Madgine/serialize/memory/memorymanager.h"
#include "Meta/serialize/operations.h"

#include "Generic/areaview.h"
#include "Generic/bytebuffer.h"

#include "Meta/math/vector3.h"

#include "Interfaces/filesystem/fsapi.h"

#include "Modules/threading/awaitables/awaitablesender.h"

#include "Madgine/serialize/filesystem/filemanager.h"

#include "Meta/serialize/container/container_operations.h"

#include "Meta/serialize/formats.h"

#undef INFINITE
#include "msdfgen.h"

#include "core/edge-coloring.h"

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
                g->mAdvance = 8192;
            return true;
        } else {
            return false;
        }
    }

    FontLoader::FontLoader()
        : ResourceLoader({ ".msdf", ".ttf" })
    {
    }

    Threading::Task<bool> FontLoader::loadImpl(Font &font, ResourceDataInfo &info)
    {

        if (info.resource()->path().extension() == ".msdf") {

            ByteBuffer fileBuffer = (co_await info.resource()->readAsync()).value();


            Memory::MemoryManager cache("msdf_cache");
            Serialize::FormattedSerializeStream in = cache.openRead(std::move(fileBuffer), Serialize::Formats::safebinary);
            assert(in);
            ByteBuffer b;
            Vector2i textureSize;
            Serialize::StreamResult result = [&]() {
                STREAM_PROPAGATE_ERROR(read(in, font.mGlyphs, nullptr));
                STREAM_PROPAGATE_ERROR(read(in, textureSize, nullptr));
                STREAM_PROPAGATE_ERROR(read(in, font.mAscender, nullptr));
                STREAM_PROPAGATE_ERROR(read(in, font.mDescender, nullptr));
                return read(in, b, nullptr);
            }();
            if (result.mState == Serialize::StreamState::OK) {
                co_return co_await font.mTexture.createTask(TextureType_2D, FORMAT_RGBA8, textureSize, std::move(b));
            }
            LOG_ERROR("Failed to load \"" << info.resource()->path() << "\": \n"
                                          << result);
            LOG("Falling back to .ttf file load");
        }
        Filesystem::Path path = info.resource()->path();
        if (path.extension() == ".msdf") {
            path = path.parentPath() / (std::string { path.stem() } + ".ttf");
        }
        assert(path.extension() == ".ttf");

        LOG("Creating Cache for " << path);

        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            LOG_ERROR("FREETYPE: Could not init FreeType Library");
            co_return false;
        }

        Stream buffer = Filesystem::openFileRead(path, true);

        std::vector<unsigned char> fontBuffer { buffer.iterator(), buffer.end() };

        FT_Face face;
        if (FT_New_Memory_Face(ft, fontBuffer.data(), fontBuffer.size(), 0, &face)) {
            FT_Done_FreeType(ft);
            LOG_ERROR("FREETYPE: Failed to load font");
            co_return false;
        }

        FT_Set_Pixel_Sizes(face, 0, 64);

        font.mAscender = face->size->metrics.ascender;
        font.mDescender = face->size->metrics.descender;

        std::array<Vector2i, Font::sFontGlyphCount> sizes;
        std::array<Vector2i, Font::sFontGlyphCount> extendedSizes;

        for (unsigned char c = 0; c < Font::sFontGlyphCount; c++) {
            if (ignore(c))
                continue;
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_DEFAULT)) {
                LOG_ERROR("FREETYTPE: Failed to load Glyph");
                sizes[c] = { 0, 0 };
                extendedSizes[c] = { 0, 0 };
                continue;
            }
            sizes[c] = { static_cast<int>(face->glyph->bitmap.width) + 2, static_cast<int>(face->glyph->bitmap.rows) + 2 };
            extendedSizes[c] = { static_cast<int>(face->glyph->bitmap.width) + 3, static_cast<int>(face->glyph->bitmap.rows) + 3 };
        }

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

        std::vector<Atlas2::Entry> entries = atlas.insert(
            extendedSizes, expand, true);

        Vector2i textureSize = { areaSize * UNIT_SIZE,
            areaSize * UNIT_SIZE };
        size_t byteSize = textureSize.x * textureSize.y;
        std::unique_ptr<std::array<unsigned char, 4>[]> texBuffer = std::make_unique<std::array<unsigned char, 4>[]>(byteSize);
        AreaView<std::array<unsigned char, 4>, 2> tex { texBuffer.get(), { static_cast<size_t>(textureSize.x), static_cast<size_t>(textureSize.y) } };

        for (unsigned char c = 0; c < Font::sFontGlyphCount; c++) {
            if (ignore(c, &font.mGlyphs[c]))
                continue;

            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                LOG_ERROR("FREETYTPE: Failed to load Glyph");
                continue;
            }

            std::unique_ptr<Vector3[]> buffer = std::make_unique<Vector3[]>(sizes[c].x * sizes[c].y);
            AreaView<Vector3, 2> bufferView { buffer.get(), { static_cast<size_t>(sizes[c].x), static_cast<size_t>(sizes[c].y) } };

            ::msdfgen::BitmapRef<float, 3>
                bm { buffer[0].ptr(), sizes[c].x, sizes[c].y };

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
            FT_Outline_Decompose(&face->glyph->outline, &ftFunctions, &context);

            ::msdfgen::edgeColoringSimple(shape, 3);
            ::msdfgen::generateMSDF(bm, shape, 4.0, { 1, 1 }, { static_cast<double>(-face->glyph->bitmap_left + 1), static_cast<double>(sizes[c].y - face->glyph->bitmap_top - 1) });

            font.mGlyphs[c].mSize = sizes[c];
            font.mGlyphs[c].mUV = entries[c].mArea.mTopLeft;
            font.mGlyphs[c].mFlipped = entries[c].mFlipped;
            font.mGlyphs[c].mAdvance = face->glyph->advance.x;
            font.mGlyphs[c].mBearing.x = face->glyph->bitmap_left - 1;
            font.mGlyphs[c].mBearing.y = face->glyph->bitmap_top - 1;

            Vector2i size = sizes[c];
            if (entries[c].mFlipped)
                std::swap(size.x, size.y);
            Vector2i pos = { entries[c].mArea.mTopLeft.x, entries[c].mArea.mTopLeft.y };

            AreaView<std::array<unsigned char, 4>, 2> targetView = tex.subArea({ static_cast<size_t>(pos.x), static_cast<size_t>(pos.y) }, { static_cast<size_t>(size.x), static_cast<size_t>(size.y) });
            if (entries[c].mFlipped)
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

        FT_Set_Pixel_Sizes(face, 0, 24);

        for (unsigned char c = 0; c < Font::sFontGlyphCount; c++) {
            if (ignore(c))
                continue;

            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_DEFAULT)) {
                LOG_ERROR("FREETYTPE: Failed to load Glyph");
                sizes[c] = { 0, 0 };
                extendedSizes[c] = { 0, 0 };
                continue;
            }
            sizes[c] = { static_cast<int>(face->glyph->bitmap.width), static_cast<int>(face->glyph->bitmap.rows) };
            extendedSizes[c] = { static_cast<int>(face->glyph->bitmap.width) + 1, static_cast<int>(face->glyph->bitmap.rows) + 1 };
        }

        entries = atlas.insert(
            extendedSizes, expand, true);

        for (unsigned char c = 0; c < Font::sFontGlyphCount; c++) {
            if (ignore(c))
                continue;

            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                LOG_ERROR("FREETYTPE: Failed to load Glyph");
                continue;
            }

            if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
                LOG_ERROR("FREETYPE: Failed to render Glyph");
                continue;
            }

            AreaView<unsigned char, 2> bufferView { face->glyph->bitmap.buffer, { static_cast<size_t>(sizes[c].x), static_cast<size_t>(sizes[c].y) } };

            font.mGlyphs[c].mSize2 = sizes[c] + Vector2i(2, 2);
            font.mGlyphs[c].mUV2 = entries[c].mArea.mTopLeft - Vector2i(1, 1);
            font.mGlyphs[c].mFlipped2 = entries[c].mFlipped;

            Vector2i size = sizes[c];
            if (entries[c].mFlipped)
                std::swap(size.x, size.y);
            Vector2i pos = { entries[c].mArea.mTopLeft.x, entries[c].mArea.mTopLeft.y };

            AreaView<std::array<unsigned char, 4>, 2> targetView = tex.subArea({ static_cast<size_t>(pos.x), static_cast<size_t>(pos.y) }, { static_cast<size_t>(size.x), static_cast<size_t>(size.y) });
            if (entries[c].mFlipped)
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

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        if (!co_await font.mTexture.createTask(TextureType_2D, FORMAT_RGBA8, textureSize, { texBuffer.get(), 4 * byteSize }))
            co_return false;

        Filesystem::FileManager cache("msdf_cache");
        Serialize::FormattedSerializeStream out = cache.openWrite(info.resource()->path().parentPath() / (std::string { info.resource()->name() } + ".msdf"), Serialize::Formats::safebinary);
        if (out) {
            write(out, font.mGlyphs, "glyphs");
            write(out, textureSize, "size");
            write(out, font.mAscender, "ascender");
            write(out, font.mDescender, "descender");
            write(out, ByteBuffer { texBuffer.get(), 4 * byteSize }, "texture");
        }

        co_return true;
    }

    void FontLoader::unloadImpl(Font &font)
    {
        font.mTexture.reset();
    }

    Threading::TaskQueue *FontLoader::loadingTaskQueue() const
    {
        return TextureLoader::getSingleton().loadingTaskQueue();
    }

}
}