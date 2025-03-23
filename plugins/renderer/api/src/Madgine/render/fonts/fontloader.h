#pragma once

#include "Madgine/resources/resourceloader.h"

#include "Modules/threading/workgroupstorage.h"

#include "font.h"

namespace Engine {
namespace Render {
	
	struct MADGINE_RENDER_EXPORT FontLoader : Resources::ResourceLoader<FontLoader, Font, std::list<Placeholder<0>>, Threading::WorkGroupStorage> {
        FontLoader();

		Threading::Task<bool> loadImpl(Font &font, ResourceDataInfo &info);
        void unloadImpl(Font &font);

		virtual Threading::TaskQueue *loadingTaskQueue() const override;
    };

}
}

REGISTER_TYPE(Engine::Render::FontLoader)