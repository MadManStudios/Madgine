#pragma once

#include "Modules/threading/workgroupstorage.h"

#include "Madgine/resources/resourceloader.h"

#include "font.h"

typedef struct FT_LibraryRec_ *FT_Library;

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT FontLoader : Resources::ResourceLoader<FontLoader, TypeFace, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        FontLoader();

        Threading::Task<bool> loadImpl(TypeFace &typeFace, ResourceDataInfo &info);
        void unloadImpl(TypeFace &typeFace);

        Threading::Task<bool> init() override;
        Threading::Task<void> finalize() override;

        Threading::TaskQueue *loadingTaskQueue() const override;

        static constexpr int sFontSize = 48;

    protected:

        std::pair<Resources::ResourceBase *, bool> addResource(const Filesystem::Path &path, std::string_view name = {}) override;

    private:
        FT_Library mFreeType;
    };

}
}