#include "../widgetslib.h"

#include "widgetloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "widget.h"
#include "button.h"
#include "scenewindow.h"
#include "label.h"
#include "image.h"
#include "layout.h"
#include "tablewidget.h"
#include "tabbar.h"
#include "textedit.h"

RESOURCELOADER(Engine::Widgets::WidgetLoader)

namespace Engine {
namespace Widgets {

    template <typename Widget>
    void registerWidget(WidgetLoader &loader, std::string_view name) {
        loader.getOrCreateManual(name, {}, [](WidgetLoader *loader, WidgetDescriptor &desc, WidgetLoader::ResourceDataInfo &info) -> Threading::Task<bool> {
            desc = type_holder<Widget>;
            co_return true;
        }, &loader);
    }

    WidgetLoader::WidgetLoader()
        : ResourceLoader({ ".widget" })
    {
        registerWidget<WidgetBase>(*this, "Widget");
        registerWidget<Button>(*this, "Button");
        registerWidget<SceneWindow>(*this, "SceneWindow");
        registerWidget<Label>(*this, "Label");
        registerWidget<Image>(*this, "Image");
        registerWidget<Layout>(*this, "Layout");
        registerWidget<TableWidget>(*this, "TableWidget");
        registerWidget<TabBar>(*this, "TabBar");
        registerWidget<TextEdit>(*this, "TextEdit");
    }

    Threading::Task<bool> WidgetLoader::loadImpl(WidgetDescriptor &descriptor, ResourceDataInfo &info)
    {
        co_return true;
    }

    Threading::Task<void> WidgetLoader::unloadImpl(WidgetDescriptor &descriptor)
    {
        co_return;
    }

    std::unique_ptr<WidgetBase> WidgetDescriptor::create(WidgetManager &manager, WidgetBase *parent) const
    {
        return mCtor(manager, parent);
    }

    const Serialize::SerializeTable *WidgetDescriptor::serializeTable()
    {
        return mSerializeTable;
    }

}
}
