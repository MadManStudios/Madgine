#pragma once

#include "Meta/math/atlas2.h"
#include "Meta/serialize/hierarchy/serializableunit.h"

#include "Madgine/imageloader/imageloader.h"
#include "Madgine/resources/resourceloader.h"

namespace Engine {
namespace Widgets {

    struct PreprocessedUIAtlas {
        SERIALIZABLEUNIT(PreprocessedUIAtlas);

        void insert(const std::map<std::string, Resources::ImageLoader::Handle> &images);

        const Math::Atlas2 &atlas() const;
        const Memory::ByteBuffer &buffer() const;
        const std::map<std::string, Math::Atlas2::Entry, std::less<>> &entries() const;
        int size() const;

        Math::Vector2i imageSize() const;

        Memory::ByteBuffer toPNG() const;
        void fromPNG(const Memory::ByteBuffer &buffer);

        void postLoad();

    private:
        void expand();

        Math::Atlas2 mAtlas { { 2048, 2048 } };
        int mSize = 0;
        std::map<std::string, Math::Atlas2::Entry, std::less<>> mEntries;
        Memory::ByteBuffer mBuffer;
    };

    struct UIAtlas {

        UIAtlas();

        void preload(const PreprocessedUIAtlas &atlas);
        void reset();

        Resources::ImageLoader::Resource *getImage(std::string_view name);

        const Math::Atlas2::Entry *lookUpImage(Resources::ImageLoader::Resource *image);
        const Math::Atlas2::Entry *lookUpImage(std::string_view name);

        Render::ResourceBlock resource();

    private:
        void expand();

        Render::TexturePtr mTexture;
        std::set<Resources::ImageLoader::Handle> mImageLoadingTasks;
        Math::Atlas2 mAtlas { { 2048, 2048 } };
        int mSize = 0;
        std::map<std::string, Math::Atlas2::Entry, std::less<>> mEntries;
        std::map<std::string, Resources::ImageLoader::Resource, std::less<>> mDummyResources;
    };

    struct MADGINE_WIDGETS_EXPORT AtlasLoader : Resources::ResourceLoader<AtlasLoader, PreprocessedUIAtlas> {

        AtlasLoader();

        Threading::Task<bool> loadImpl(PreprocessedUIAtlas &data, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(PreprocessedUIAtlas &data);

        virtual Threading::Task<Resources::BakeResult> bakeResources(std::vector<Platform::Filesystem::Path> &resourcesToBake, const Platform::Filesystem::Path &intermediateDir) override;
    };

}
}