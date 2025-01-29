#include "../widgetslib.h"

#include "combobox.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN_BASE(Engine::Widgets::Combobox, Engine::Widgets::WidgetBase)
METATABLE_END(Engine::Widgets::Combobox)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Combobox, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::Combobox)

namespace Engine {
namespace Widgets {

    void Combobox::addItem(const std::string &text)
    {
    }

    void Combobox::clear()
    {
    }

    void Combobox::setText(const std::string &s)
    {
    }

    std::string Combobox::getText()
    {
        return std::string();
    }

    WidgetClass Combobox::getClass() const
    {
        return WidgetClass::COMBOBOX;
    }

}
}