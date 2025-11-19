#pragma once

#include "Modules/threading/madgineobject.h"

#include "Meta/keyvalue/virtualscope.h"

#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"

typedef int ImGuiWindowFlags;

namespace Engine {
namespace Tools {

    struct Tip {
        std::string mTitle;
        std::string mText;
    };

    struct MADGINE_TOOLS_EXPORT ToolBase : Serialize::VirtualSerializableUnitBase<VirtualScopeBase<>, Serialize::SerializableUnitBase>, Threading::MadgineObject<ToolBase> {
        SERIALIZABLEUNIT(ToolBase)

        ToolBase(ImRoot &root);
        virtual ~ToolBase() = default;

        virtual void render();
        virtual void renderMenu();
        virtual bool renderConfiguration(const Filesystem::Path &config);
        virtual void renderSettings();
        virtual void renderMetrics();
        virtual void update();

        virtual void loadConfiguration(const Filesystem::Path &config);
        virtual void saveConfiguration(const Filesystem::Path &config);

        virtual std::vector<Tip> tips();

        virtual std::string_view key() const = 0;

        bool isVisible() const;
        void setVisible(bool v);

        bool isEnabled() const;
        void setEnabled(bool e);

        ToolBase &getTool(size_t index);
        template <typename T>
        T &getTool()
        {
            return static_cast<T &>(getTool(UniqueComponent::component_index<T>()));
        }

        ImRoot &root();

        Threading::TaskQueue *taskQueue() const;

        ImGuiID dockSpaceId() const;

    protected:
        bool beginTool(const char *name, bool *open, ImGuiDir dockingDir, ImGuiWindowFlags flags = 0, const char *docTarget = nullptr, const char *pluginSourceDir = PROJECT_ROOT);
        bool beginToolWindow(const char *name, bool *open, ImGuiWindowFlags flags = 0, const char *docTarget = nullptr, const char *pluginSourceDir = PROJECT_ROOT);
        bool beginToolPanel(const char *name, bool *open, ImGuiDir dockingDir, ImGuiWindowFlags flags = 0, const char *docTarget = nullptr, const char *pluginSourceDir = PROJECT_ROOT);
        bool beginSubPanel(const char *name, bool *open, ImGuiDir dockingDir, float ratio = 0.2f, ImGuiWindowFlags flags = 0);
        bool beginContent(ImGuiWindowFlags flags = 0);
        bool beginGamePanel(const char *name, bool *open, ImGuiDir dockingDir, float ratio = 0.2f, ImGuiWindowFlags flags = 0);

    protected:
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();
        friend struct Threading::MadgineObject<ToolBase>;

        bool mVisible = false;

        bool mEnabled = true;

        ImGuiID mDockSpaceId = 0;

        ImRoot &mRoot;
    };

}
}