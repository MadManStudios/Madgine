#pragma once

#include "../toolscollector.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "dialogs.h"

#include "Interfaces/filesystem/path.h"

namespace Engine {

struct MadgineObjectState;

namespace Tools {

    struct MADGINE_TOOLS_EXPORT ImRoot {
        ImRoot();
        ~ImRoot();

        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();

        const std::vector<std::unique_ptr<ToolBase>> &tools();
        ToolBase &getTool(size_t index);
        template <typename T>
        T &getTool()
        {
            return static_cast<T &>(getTool(::Engine::UniqueComponent::component_index<T>()));
        }

        void render();

        unsigned int dockSpaceId() const;

        std::stringstream mToolReadBuffer;
        ToolBase *mToolReadTool = nullptr;

        void finishToolRead();

        virtual Threading::TaskQueue *taskQueue() const = 0;

        DialogContainer &dialogs();

        Dialog<Filesystem::Path> directoryPicker(Filesystem::Path path = {}, Filesystem::Path selected = {});
        Dialog<Filesystem::Path> filePicker(bool allowNewFile = false, Filesystem::Path path = {}, Filesystem::Path selected = {});


    protected:        
        unsigned int mDockSpaceId;

    private:
        ToolsContainer<std::vector<Placeholder<0>>> mCollector;

        DialogContainer mDialogContainer;
    };

}
}

REGISTER_TYPE(Engine::Tools::ImRoot)
