#pragma once

#include "Modules/threading/madgineobject.h"

#include "Meta/keyvalue/virtualscope.h"

#include "Meta/serialize/hierarchy/virtualserializableunit.h"

#include "Modules/uniquecomponent/uniquecomponent.h"

namespace Engine {
namespace Tools {

    struct MADGINE_TOOLS_EXPORT ToolBase : Serialize::VirtualSerializableUnitBase<VirtualScopeBase<>, Serialize::SerializableUnitBase>, Threading::MadgineObject<ToolBase> {
        SERIALIZABLEUNIT(ToolBase)

        ToolBase(ImRoot &root, bool visible = false);
        virtual ~ToolBase() = default;

        virtual void render();
        virtual void renderMenu();
        virtual void renderStatus();
        virtual bool renderConfiguration(const Filesystem::Path &config);
        virtual void renderSettings();
        virtual void renderMetrics();
        virtual void update();

        virtual void loadConfiguration(const Filesystem::Path &config);
        virtual void saveConfiguration(const Filesystem::Path &config);

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
        virtual Threading::Task<bool> init();
        virtual Threading::Task<void> finalize();
        friend struct MadgineObject<ToolBase>;

        bool mVisible = false;

        bool mEnabled = true;

        ImRoot &mRoot;
    };

}
}

REGISTER_TYPE(Engine::Tools::ToolBase)