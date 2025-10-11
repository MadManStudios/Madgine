#pragma once

#include "../toolscollector.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "dialogs.h"

#include "Interfaces/filesystem/path.h"

#include "Meta/math/vector2i.h"

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

        bool render();

        unsigned int rootDockSpaceId() const;
        unsigned int gameDockSpaceId() const;

        std::stringstream mToolReadBuffer;
        ToolBase *mToolReadTool = nullptr;

        void finishToolRead();

        virtual Filesystem::Path findDataFile(std::string_view name) const = 0;

        virtual Threading::TaskQueue *taskQueue() const = 0;

        DialogContainer &dialogs();

        Dialog<Filesystem::Path> directoryPicker(Filesystem::Path path = {}, Filesystem::Path selected = {});
        Dialog<Filesystem::Path> filePicker(bool allowNewFile = false, Filesystem::Path path = {}, Filesystem::Path selected = {});

        virtual void Image(const Filesystem::Path &path, Vector2i image_size = { -1, -1 }) = 0;
        virtual void DrawImage(const Filesystem::Path &path, Vector2i pos, Vector2i image_size = { -1, -1 }, float spinnerRadius = 15) = 0;

        std::map<unsigned int, size_t> mDockSpaces;

    protected:        
        unsigned int mRootDockSpaceId;
        unsigned int mGameDockSpaceId;

    private:
        ToolsContainer<std::vector<Placeholder<0>>> mCollector;

        DialogContainer mDialogContainer;
    };

}
}
