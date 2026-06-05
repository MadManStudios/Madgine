#include "../rootlib.h"

#include "rootcomponentcollector.h"

#include "Modules/uniquecomponent/uniquecomponentregistry.h"

#include "root.h"

DEFINE_UNIQUE_COMPONENT(Engine::Core, RootComponent)

namespace Engine {
namespace Core {

    RootComponentBase &getRootComponent(size_t i)
    {
        return Root::getSingleton().getComponent(i);
    }

}
}
