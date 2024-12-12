#include "../widgetslib.h"

#include "atlasloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Madgine/serialize/filesystem/filemanager.h"

#include "Meta/serialize/streams/formattedserializestream.h"

#include "Meta/serialize/formats.h"

#include "Meta/serialize/operations.h"

#include "widgetmanager.h"

#include "Madgine/window/mainwindow.h"

#include "Madgine/window/layoutloader.h"

#include "Generic/areaview.h"

RESOURCELOADER(Engine::Widgets::AtlasLoader)

SERIALIZETABLE_BEGIN(Engine::Widgets::PreprocessedUIAtlas)
FIELD(mSize)
FIELD(mEntries, Engine::Serialize::KeyValueCreator<Engine::Serialize::DefaultCreator, Engine::Serialize::DefaultCreator>)
ENCAPSULATED_FIELD(Image, toPNG, fromPNG)
SERIALIZETABLE_END(Engine::Widgets::PreprocessedUIAtlas)

namespace Engine {
namespace Widgets {

    UIAtlas::UIAtlas()
    {
    }

    Threading::Task<bool> UIAtlas::createTexture()
    {
        return mTexture.createTask(Render::TextureType_2D, Render::FORMAT_RGBA8_SRGB);
    }

    void UIAtlas::preload(const PreprocessedUIAtlas &atlas)
    {
        mEntries = atlas.entries();
        mSize = atlas.size();
        if (mSize > 0) {
            mTexture.setData(atlas.imageSize(), atlas.buffer());
        }
        mAtlas = atlas.atlas();
        for (const std::string &entry : kvKeys(mEntries)) {
            mDummyResources.try_emplace(entry, entry);
        }
    }

    void UIAtlas::reset()
    {
        mTexture.reset();
    }

    Resources::ImageLoader::Resource *UIAtlas::getImage(std::string_view name)
    {
        if (mDummyResources.contains(name))
            return &mDummyResources.find(name)->second;
        return Resources::ImageLoader::get(name);
    }

    const Atlas2::Entry *UIAtlas::lookUpImage(Resources::ImageLoader::Resource *image)
    {
        if (!image)
            return nullptr;
        auto it = mEntries.find(image->name());
        if (it == mEntries.end()) {
            Resources::ImageLoader::Handle data = image->loadData();
            if (!data.available()) {
                mImageLoadingTasks.emplace(std::move(data));
                return nullptr;
            } else {

                size_t width = data->mSize.x;
                size_t height = data->mSize.y;

                assert(data->mChannels == 4);

                const uint32_t *source = static_cast<const uint32_t *>(data->mBuffer.mData);

                std::vector<uint32_t> targetBuffer;
                targetBuffer.resize((width + 2) * (height + 2));

                AreaView<uint32_t, 2> targetArea { targetBuffer.data(), { width + 2, height + 2 } };

                AreaView<const uint32_t, 2> sourceArea { source, { width, height } };

                std::ranges::copy(sourceArea, targetArea.subArea({ 1, 1 }, { width, height }).begin());
                std::ranges::copy(sourceArea.subArea({ 0, 0 }, { 1, height }), targetArea.subArea({ 0, 1 }, { 1, height }).begin());
                std::ranges::copy(sourceArea.subArea({ width - 1, 0 }, { 1, height }), targetArea.subArea({ width + 1, 1 }, { 1, height }).begin());
                std::ranges::copy(sourceArea.subArea({ 0, 0 }, { width, 1 }), targetArea.subArea({ 1, 0 }, { width, 1 }).begin());
                std::ranges::copy(sourceArea.subArea({ 0, height - 1 }, { width, 1 }), targetArea.subArea({ 1, height + 1 }, { width, 1 }).begin());

                targetArea[0][0] = sourceArea[0][0];
                targetArea[0][width + 1] = sourceArea[0][width - 1];
                targetArea[height + 1][0] = sourceArea[height - 1][0];
                targetArea[height + 1][width + 1] = sourceArea[height - 1][width - 1];

                Atlas2::Entry entry = mAtlas.insert(data->mSize + Vector2i { 2, 2 }, [this]() { expand(); });
                it = mEntries.try_emplace(std::string { image->name() }, entry).first;
                mTexture.setSubData(entry.mArea.mTopLeft, data->mSize + Vector2i { 2, 2 }, targetBuffer);
                mImageLoadingTasks.erase(data);
            }
        }
        return &it->second;
    }

    const Atlas2::Entry *UIAtlas::lookUpImage(std::string_view name)
    {
        return lookUpImage(Resources::ImageLoader::get(name));
    }

    Render::ResourceBlock UIAtlas::resource()
    {
        return mTexture->resource();
    }

    void UIAtlas::expand()
    {
        if (mSize == 0) {
            mSize = 1;
            mTexture.setData({ mSize * 2048, mSize * 2048 }, {});
            for (int x = 0; x < mSize; ++x) {
                for (int y = 0; y < mSize; ++y) {
                    mAtlas.addBin({ 2048 * x, 2048 * y });
                }
            }
        } else {
            /*for (int x = 0; x < mUIAtlasSize; ++x) {
                for (int y = 0; y < mUIAtlasSize; ++y) {
                    mUIAtlas.addBin({ 512 * x, 512 * (y + mUIAtlasSize) });
                    mUIAtlas.addBin({ 512 * (x + mUIAtlasSize), 512 * y });
                    mUIAtlas.addBin({ 512 * (x + mUIAtlasSize), 512 * (y + mUIAtlasSize) });
                }
            }
            mUIAtlasSize *= 2;
            mUIAtlasTexture.resize({ 512 * mUIAtlasSize, 512 * mUIAtlasSize });*/
            throw "TODO";
        }
    }

    void PreprocessedUIAtlas::insert(const std::map<std::string, Resources::ImageLoader::Handle> &images)
    {
        std::vector<Vector2i> sizes { images.size() };
        size_t i = 0;
        for (const auto &[name, handle] : images) {
            sizes[i] = handle->mSize + Vector2i { 2, 2 };
            ++i;
        }
        std::vector<Atlas2::Entry> entries = mAtlas.insert({sizes.data(), sizes.size()}, [this]() { expand(); });
        i = 0;
        for (const auto &[name, handle] : images) {
            mEntries[name] = entries[i];
            ++i;
        }
        size_t imageSize = mSize * 2048;
        size_t bufferSize = imageSize * imageSize;
        ByteBufferImpl<uint32_t> atlasBuffer { std::make_unique<uint32_t[]>(bufferSize), bufferSize };
        AreaView<uint32_t, 2> bufferView { atlasBuffer.mData, { imageSize, imageSize } };
        for (const auto &[name, entry] : mEntries) {

            size_t width = entry.mArea.mSize.x - 2;
            size_t height = entry.mArea.mSize.y - 2;

            AreaView<uint32_t, 2> targetArea = bufferView.subArea(
                { static_cast<size_t>(entry.mArea.mTopLeft.x), static_cast<size_t>(entry.mArea.mTopLeft.y) },
                { static_cast<size_t>(entry.mArea.mSize.x), static_cast<size_t>(entry.mArea.mSize.y) });

            AreaView<const uint32_t, 2> sourceArea { static_cast<const uint32_t *>(images.at(name)->mBuffer.mData), { width, height } };

            std::ranges::copy(sourceArea, targetArea.subArea({ 1, 1 }, { width, height }).begin());
            std::ranges::copy(sourceArea.subArea({ 0, 0 }, { 1, height }), targetArea.subArea({ 0, 1 }, { 1, height }).begin());
            std::ranges::copy(sourceArea.subArea({ width - 1, 0 }, { 1, height }), targetArea.subArea({ width + 1, 1 }, { 1, height }).begin());
            std::ranges::copy(sourceArea.subArea({ 0, 0 }, { width, 1 }), targetArea.subArea({ 1, 0 }, { width, 1 }).begin());
            std::ranges::copy(sourceArea.subArea({ 0, height - 1 }, { width, 1 }), targetArea.subArea({ 1, height + 1 }, { width, 1 }).begin());

            targetArea[0][0] = sourceArea[0][0];
            targetArea[0][width + 1] = sourceArea[0][width - 1];
            targetArea[height + 1][0] = sourceArea[height - 1][0];
            targetArea[height + 1][width + 1] = sourceArea[height - 1][width - 1];
        }
        mBuffer = std::move(atlasBuffer).cast<const void>();
    }

    const Atlas2 &PreprocessedUIAtlas::atlas() const
    {
        return mAtlas;
    }

    const ByteBuffer &PreprocessedUIAtlas::buffer() const
    {
        return mBuffer;
    }

    const std::map<std::string, Atlas2::Entry, std::less<>> &PreprocessedUIAtlas::entries() const
    {
        return mEntries;
    }

    int PreprocessedUIAtlas::size() const
    {
        return mSize;
    }

    Vector2i PreprocessedUIAtlas::imageSize() const
    {
        return { mSize * 2048, mSize * 2048 };
    }

    ByteBuffer PreprocessedUIAtlas::toPNG() const
    {
        return Resources::ImageLoader::convertToPNG(mBuffer, imageSize());        
    }

    void PreprocessedUIAtlas::fromPNG(const ByteBuffer &buffer)
    {
        Vector2i size;
        mBuffer = Resources::ImageLoader::convertFromPNG(buffer, size);
    }

    void PreprocessedUIAtlas::postLoad()
    {
        mAtlas.clear();
        mAtlas.addBin({ 0, 0 });
        Vector2i furthestCorner { 0, 0 };
        for (const Atlas2::Entry& entry : kvValues(mEntries)) {
            Vector2i bottomRight = entry.mArea.bottomRight();
            furthestCorner = { std::max(bottomRight.x, furthestCorner.x),
                std::max(bottomRight.y, furthestCorner.y) };
        }
        mAtlas.insert(furthestCorner, {}, false);
    }

    void PreprocessedUIAtlas::expand()
    {
        if (mSize == 0) {
            mSize = 1;
            //mTexture.setData({ mSize * 2048, mSize * 2048 }, {});
            for (int x = 0; x < mSize; ++x) {
                for (int y = 0; y < mSize; ++y) {
                    mAtlas.addBin({ 2048 * x, 2048 * y });
                }
            }
        } else {
            /*for (int x = 0; x < mUIAtlasSize; ++x) {
                for (int y = 0; y < mUIAtlasSize; ++y) {
                    mUIAtlas.addBin({ 512 * x, 512 * (y + mUIAtlasSize) });
                    mUIAtlas.addBin({ 512 * (x + mUIAtlasSize), 512 * y });
                    mUIAtlas.addBin({ 512 * (x + mUIAtlasSize), 512 * (y + mUIAtlasSize) });
                }
            }
            mUIAtlasSize *= 2;
            mUIAtlasTexture.resize({ 512 * mUIAtlasSize, 512 * mUIAtlasSize });*/
            throw "TODO";
        }
    }

    AtlasLoader::AtlasLoader()
        : ResourceLoader({ ".atl" })
    {
    }

    Threading::Task<bool> AtlasLoader::loadImpl(PreprocessedUIAtlas &data, ResourceDataInfo &info)
    {
        Serialize::SerializeManager mgr { "Atlas" };
        Serialize::FormattedSerializeStream stream { Serialize::Formats::safebinary(), mgr.wrapStream(info.resource()->readAsStream(true), true) };
        Serialize::StreamResult result = Serialize::read(stream, data, "Atlas");
        if (result.mState != Serialize::StreamState::OK) {
            LOG_ERROR("Failed loading Atlas " << info.resource()->path());
            co_return false;
        }
        data.postLoad();
        co_return true;
    }

    Threading::Task<void> AtlasLoader::unloadImpl(PreprocessedUIAtlas &data)
    {       
        co_return;
    }

    Threading::Task<Resources::BakeResult> AtlasLoader::bakeResources(std::vector<Filesystem::Path> &resourcesToBake, const Filesystem::Path &intermediateDir)
    {
        std::vector<Filesystem::Path> filesToAdd;
        for (Filesystem::Path path : resourcesToBake) {
            if (path.extension() == ".layout") {
                if (path.isRelative())
                    path = Filesystem::Path { SOURCE_DIR } / path;
                Window::LayoutLoader::Resource res { "", path };
                Serialize::SerializeManager mgr { "Layout" };
                Serialize::FormattedSerializeStream stream = res.readAsFormattedStream(mgr);
                std::map<std::string, Resources::ImageLoader::Handle> images;
                Serialize::StreamResult result = Serialize::scanCompound<Window::MainWindow, WidgetManager>(stream, nullptr, [&](Serialize::FormattedSerializeStream &stream, const char *name) {
                    return Serialize::scanPrimitive<WidgetManager, std::string>(stream, name, [&](const std::string &s, std::span<std::string_view> tags) {
                        if (!s.empty() && std::ranges::contains(tags, "Image"))
                            images[s];
                    });
                });
                if (result.mState != Serialize::StreamState::OK) {
                    LOG_ERROR(*result.mError);
                    co_return Resources::BakeResult::UNKNOWN_ERROR;
                }
                for (auto &[name, handle] : images)
                    handle.load(name);

                for (const auto &[name, handle] : images) {
                    if (!handle) {
                        LOG_ERROR("Image not found: '" << name << "'!");
                        co_return Resources::BakeResult::UNKNOWN_ERROR;
                    }
                    if (!co_await handle.info()->loadingTask()) {
                        LOG_ERROR("Failed to load image '" << name << "'!");
                        co_return Resources::BakeResult::UNKNOWN_ERROR;
                    }
                }
                LOG("Creating UI Atlas containing " << images.size() << " images...");
                PreprocessedUIAtlas atlas;
                atlas.insert(images);
                Filesystem::FileManager outMgr { "Atlas" };
                Filesystem::Path outPath = intermediateDir / (std::string { path.stem() } + ".atl");
                Serialize::FormattedSerializeStream out = outMgr.openWrite(outPath, Serialize::Formats::safebinary);
                
                Serialize::write(out, atlas, "Atlas");   

                filesToAdd.push_back(outPath);
            }
        }
        std::ranges::move(filesToAdd, std::back_inserter(resourcesToBake));
        co_return Resources::BakeResult::SUCCESS;
    }

}
}
