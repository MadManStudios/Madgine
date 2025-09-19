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

        ToolBase(ImRoot &root, bool visible = false);
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

    protected:
        bool beginDefaultWindow(ImGuiDir dockingDir = ImGuiDir_None, const char *docTarget = nullptr, ImGuiWindowFlags flags = 0, const char *pluginSourceDir = PROJECT_ROOT);

    protected:
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();
        friend struct Threading::MadgineObject<ToolBase>;

        bool mVisible = false;

        bool mEnabled = true;

        ImRoot &mRoot;
    };

}
}