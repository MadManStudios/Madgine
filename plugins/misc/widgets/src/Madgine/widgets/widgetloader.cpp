#include "../widgetslib.h"

#include "widgetloader.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "Meta/serialize/formats.h"
#include "Meta/serialize/serializemanager.h"

#include "button.h"
#include "image.h"
#include "label.h"
#include "layout.h"
#include "scenewindow.h"
#include "tabbar.h"
#include "tablewidget.h"
#include "textedit.h"
#include "widget.h"

RESOURCELOADER(Engine::Widgets::WidgetLoader)

namespace Engine {
namespace Widgets {

    template <typename Widget>
    void registerWidget(WidgetLoader &loader, std::string_view name)
    {
        loader.getOrCreateManual(name, {}, [](WidgetLoader *loader, WidgetDescriptor &desc, WidgetLoader::ResourceDataInfo &info) -> Threading::Task<bool> {
            desc = type_holder<Widget>;
            co_return true; }, &loader);
    }

    WidgetLoader::WidgetLoader()
        : ResourceLoader({ ".widget" }, { .mAutoLoad = true })
    {
    }

    Threading::Task<bool> WidgetLoader::init()
    {
        if (!co_await ResourceLoaderBase::init())
            co_return false;

        registerWidget<WidgetBase>(*this, "Widget");
        registerWidget<Button>(*this, "Button");
        registerWidget<SceneWindow>(*this, "SceneWindow");
        registerWidget<Label>(*this, "Label");
        registerWidget<Image>(*this, "Image");
        registerWidget<Layout>(*this, "Layout");
        registerWidget<TableWidget>(*this, "TableWidget");
        registerWidget<TabBar>(*this, "TabBar");
        registerWidget<TextEdit>(*this, "TextEdit");

        co_return true;
    }

    Threading::Task<bool> WidgetLoader::loadImpl(WidgetDescriptor &descriptor, ResourceDataInfo &info)
    {
        Serialize::SerializeManager mgr { "Atlas" };
        Serialize::FormattedSerializeStream stream { Serialize::Formats::xml(), mgr.wrapStream(info.resource()->readAsStream(), true) };

        WidgetTemplate _template;

        Serialize::StreamResult result = Serialize::read(stream, _template, "Widget");
        if (result.mState != Serialize::StreamState::OK) {
            LOG_ERROR(result);
            co_return false;
        }

        descriptor = {
            [](WidgetManager &mgr, WidgetBase *parent, const WidgetDescriptor *desc) -> std::unique_ptr<WidgetBase> {
                return std::make_unique<CompoundWidget>(mgr, desc->widgetTemplate(), parent);
            },
            nullptr,
            std::move(_template)
        };

        co_return true;
    }

    Threading::Task<void> WidgetLoader::unloadImpl(WidgetDescriptor &descriptor)
    {
        co_return;
    }

    std::unique_ptr<WidgetBase> WidgetDescriptor::create(WidgetManager &manager, WidgetBase *parent) const
    {
        return mCtor(manager, parent, this);
    }

    const Serialize::SerializeTable *WidgetDescriptor::serializeTable()
    {
        return mSerializeTable;
    }

    const WidgetTemplate &WidgetDescriptor::widgetTemplate() const
    {
        return mTemplate;
    }
}
}
