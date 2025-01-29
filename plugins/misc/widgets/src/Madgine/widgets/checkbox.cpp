#include "../widgetslib.h"

#include "checkbox.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN_BASE(Engine::Widgets::Checkbox, Engine::Widgets::WidgetBase)
METATABLE_END(Engine::Widgets::Checkbox)

SERIALIZETABLE_INHERIT_BEGIN(Engine::Widgets::Checkbox, Engine::Widgets::WidgetBase)
SERIALIZETABLE_END(Engine::Widgets::Checkbox)

namespace Engine {
namespace Widgets {

    bool Checkbox::isChecked()
    {
        return false;
    }

    void Checkbox::setChecked(bool b)
    {
    }

    WidgetClass Checkbox::getClass() const
    {
        return WidgetClass::CHECKBOX;
    }

}
}