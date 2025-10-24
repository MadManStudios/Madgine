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

#include "widgetmanager.h"

#include "Meta/serialize/helper/typedobjectserialize.h"

#include "compoundwidget.h"

#include "widgetcollector.h"

#include "widgettemplate.h"

RESOURCELOADER(Engine::Widgets::WidgetLoader)

SERIALIZETABLE_BEGIN(Engine::Widgets::WidgetTemplate)
SERIALIZETABLE_END(Engine::Widgets::WidgetTemplate)

namespace Engine {
namespace Widgets {

    WidgetLoader::Handle::Handle() = default;

    WidgetLoader::Handle::Handle(Resource *res)
        : Base::Handle(res)
    {
    }

    WidgetLoader::WidgetLoader()
        : ResourceLoader({ ".widget" }, { .mAutoLoad = true, .mIconName = "WidgetIcon.png" })
    {
    }

    Threading::Task<bool> WidgetLoader::init()
    {
        if (!co_await ResourceLoaderBase::init())
            co_return false;

        /* getOrCreateManual("Widget", {}, [](WidgetLoader *loader, WidgetDescriptor &desc, WidgetLoader::ResourceDataInfo &info) -> Threading::Task<bool> {
                desc = type_holder<WidgetBase>;
                co_return true; }, this);            */

        co_return true;
    }

    Threading::Task<bool> WidgetLoader::loadImpl(WidgetDescriptor &descriptor, ResourceDataInfo &info)
    {
        std::vector<unsigned char> blob = info.resource()->readAsBlob();

        Serialize::SerializeManager mgr { "WidgetLoader" };
        Serialize::FormattedSerializeStream stream { Serialize::Formats::xml(), mgr.wrapStream(info.resource()->readAsStream(), true) };

        std::vector<WidgetData> widgets;

        Serialize::StreamVisitorImpl visitor {
            [&](Serialize::PrimitiveHolder<Serialize::DataTag> holder, Serialize::FormattedSerializeStream &in, const char *name, std::span<std::string_view> tags, size_t depth) -> std::optional<Serialize::StreamResult> {
                if (depth != 2) {
                    return {};
                }
                WidgetData &data = widgets.emplace_back();
                data.mType = strrchr(holder.mTable->mTypeName, ':') + 1;
                return Serialize::scanPrimitive<std::string>(holder, stream, name, [&](const std::string &s, const char *name, std::span<std::string_view> tags, size_t depth) {
                    if (depth == 1 && std::string_view { name } == "mName") {
                        data.mName = s;
                    }
                });
            }
        };

        Serialize::StreamResult result = Serialize::visitStream<Widgets::WidgetBase>(stream, "Widget", visitor);

        if (result.mState != Serialize::StreamState::OK) {
            LOG_ERROR(result);
            co_return false;
        }

        descriptor = {
            std::make_unique<WidgetTemplate>(std::string { info.resource()->name() }, std::move(widgets))
        };

        co_return true;
    }

    Threading::Task<void> WidgetLoader::unloadImpl(WidgetDescriptor &descriptor)
    {
        co_return;
    }

    WidgetDescriptor::WidgetDescriptor(std::unique_ptr<WidgetTemplate> _template)
        : mMetaTable(&_template->mMetaTable)
        , mTemplate(std::move(_template))
    {
    }

    WidgetDescriptor::~WidgetDescriptor() = default;

    WidgetDescriptor &WidgetDescriptor::operator=(WidgetDescriptor &&) noexcept = default;

    const MetaTable *WidgetDescriptor::metaTable()
    {
        return mMetaTable;
    }

    std::unique_ptr<WidgetBase> WidgetLoader::Handle::create(WidgetManager &manager, WidgetBase *parent) const
    {
        const WidgetDescriptor &desc = **this;
        if (desc.widgetTemplate()) {
            return std::make_unique<CompoundWidget>(manager, *this, parent);
        } else {
            return desc.create(manager, *this, parent);
        }
    }

    const std::unique_ptr<WidgetTemplate> &WidgetDescriptor::widgetTemplate() const
    {
        return mTemplate;
    }

    std::unique_ptr<WidgetBase> WidgetDescriptor::create(WidgetManager &manager, WidgetLoader::Handle handle, WidgetBase *parent) const
    {
        return mCtor(manager, parent, std::move(handle));
    }
}
}
