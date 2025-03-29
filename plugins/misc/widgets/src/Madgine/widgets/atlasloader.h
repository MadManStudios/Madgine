#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Madgine/imageloader/imageloader.h"
#include "Madgine/render/textureloader.h"

#include "Meta/math/atlas2.h"

#include "Meta/serialize/hierarchy/serializableunit.h"

namespace Engine {
namespace Widgets {
       
    struct PreprocessedUIAtlas {
        SERIALIZABLEUNIT(PreprocessedUIAtlas);

        void insert(const std::map<std::string, Resources::ImageLoader::Handle> &images);

        const Atlas2 &atlas() const;
        const ByteBuffer &buffer() const;
        const std::map<std::string, Atlas2::Entry, std::less<>> &entries() const;
        int size() const;

        Vector2i imageSize() const;

        ByteBuffer toPNG() const;
        void fromPNG(const ByteBuffer &buffer);

        void postLoad();

    private:
        void expand();

        Atlas2 mAtlas { { 2048, 2048 } };
        int mSize = 0;
        std::map<std::string, Atlas2::Entry, std::less<>> mEntries;
        ByteBuffer mBuffer;
    };

     struct UIAtlas {

        UIAtlas();

        Threading::Task<bool> createTexture();
        void preload(const PreprocessedUIAtlas &atlas);
        void reset();

        Resources::ImageLoader::Resource *getImage(std::string_view name);

        const Atlas2::Entry *lookUpImage(Resources::ImageLoader::Resource *image);
        const Atlas2::Entry *lookUpImage(std::string_view name);

        Render::ResourceBlock resource();

    private:
        void expand();

        Render::TextureLoader::Ptr mTexture;
        std::set<Resources::ImageLoader::Handle> mImageLoadingTasks;
        Atlas2 mAtlas { { 2048, 2048 } };
        int mSize = 0;
        std::map<std::string, Atlas2::Entry, std::less<>> mEntries;
        std::map<std::string, Resources::ImageLoader::Resource, std::less<>> mDummyResources;
    };


    struct MADGINE_WIDGETS_EXPORT AtlasLoader : Resources::ResourceLoader<AtlasLoader, PreprocessedUIAtlas> {

        AtlasLoader();

        Threading::Task<bool> loadImpl(PreprocessedUIAtlas &data, ResourceDataInfo &info);
        Threading::Task<void> unloadImpl(PreprocessedUIAtlas &data);

        virtual Threading::Task<Resources::BakeResult> bakeResources(std::vector<Filesystem::Path> &resourcesToBake, const Filesystem::Path &intermediateDir) override;
    };

}
}

REGISTER_TYPE(Engine::Widgets::AtlasLoader)