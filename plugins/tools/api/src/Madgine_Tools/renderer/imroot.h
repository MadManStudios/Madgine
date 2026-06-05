#pragma once

#include "Platform/filesystem/path.h"

#include "Meta/math/vector2.h"

#include "Modules/uniquecomponent/uniquecomponentcontainer.h"

#include "Madgine/render/renderforward.h"

#include "../toolscollector.h"
#include "dialogs.h"

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
            return static_cast<T &>(getTool(::Engine::Plugins::component_index<T>()));
        }

        bool render();

        unsigned int rootDockSpaceId() const;
        unsigned int gameDockSpaceId() const;

        std::stringstream mToolReadBuffer;
        ToolBase *mToolReadTool = nullptr;

        void finishToolRead();

        virtual Platform::Filesystem::Path findDataFile(std::string_view name) const = 0;

        virtual Threading::TaskQueue *taskQueue() const = 0;

        DialogContainer &dialogs();

        Dialog<> closeDialog();

        Dialog<Platform::Filesystem::Path> directoryPicker(Platform::Filesystem::Path path = {}, Platform::Filesystem::Path selected = {}, Platform::Filesystem::Path base = {});
        Dialog<Platform::Filesystem::Path> filePicker(bool allowNewFile = false, Platform::Filesystem::Path path = {}, Platform::Filesystem::Path selected = {});

        virtual void Image(Render::ConstTexturePtr tex, Math::Vector2i image_size = { -1, -1 }, const Math::Vector2 &uv0 = { 0, 0 }, const Math::Vector2 &uv1 = { 1, 1 }) = 0;
        virtual void Image(const Platform::Filesystem::Path &path, Math::Vector2i image_size = { -1, -1 }) = 0;
        virtual void DrawImage(const Platform::Filesystem::Path &path, Math::Vector2i pos, Math::Vector2i image_size = { -1, -1 }, float spinnerRadius = 15) = 0;

        ::ImGuiTestEngine *testEngine() const;

        std::map<unsigned int, size_t> mDockSpaces;

    protected:
        void renderDragDropTooltips();

        unsigned int mRootDockSpaceId;
        unsigned int mGameDockSpaceId;

    private:
        ToolsContainer<std::vector<Placeholder<0>>> mCollector;

        DialogContainer mDialogContainer;

        ::ImGuiTestEngine *mTestEngine = nullptr;
    };

}
}
